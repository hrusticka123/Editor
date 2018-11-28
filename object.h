#ifndef OBJECT_H
#define OBJECT_H

#include <QPainter>
#include <vector>

//vsetky class vykreslovanych objektov aj s ich vlastnostami

typedef std::vector<QPoint> Body;

//enum na jednotlive nastroje
enum Tool {enTr, enRec, enCir, enLi, enNone, enPick};

//pretazenie operatora, aby sme mohli narabat s enumom v streame
static QDataStream& operator>>( QDataStream& in, Tool& tool) {
    quint32 buffer;
    in >> buffer;
    tool = Tool(buffer);

    return in;
}

//otcovska class z ktorej nasledne konkretne objekty dedia
class Object
{
protected:
    int width_;
    //body definujuce objekt
    Body borders_;
    QColor color_;
    Tool tool_;
    bool selected_ = false;
    qreal rotation_ = 0;
    bool filled_ = false;

public:
    //Get fcie vracaju hodnoty objektu
    Body GetBorders() const { return borders_; };
    QColor GetColor() const { return color_; };
    int GetWidth() const { return width_; };
    Tool GetTool() const { return tool_; };
    bool GetSelect() const {return selected_; };
    qreal GetRotation() const {return rotation_; };
    bool GetFilled() const { return filled_; };

    //Set fcie nastavujú niektoré z nich
    void Select(bool select) { selected_ = select; };
    void SetRotation(qreal r) { rotation_ = r; };
    void SetColor(QColor c) { color_ = c; };
    void SetWidth(int w) { width_ = w; };
    void SetFilled(bool f) { filled_ = f; };
    //nastavenie nových hraníc
    void SetBorder(QPoint p, int i) {
        borders_[i] = p;
        SetPosition(0,0);
    };

    //ak sa bod p nachadza nad vykreslenym danym objektom, vracia true
    //kazdy objekt ma tuto fciu inu
    virtual bool pointIsOver(QPoint) const { return false; };

    //offsetuje body definujuce objekt, uklada ich a upravi podla nich samotny objekt a jeho stred
    virtual void SetPosition(int, int) {};

    //spolocna fcia ktora vrati true ak je bod toKnow na usecke definovanej bodmi first a second
    bool isOnLine(const QPoint &first, const QPoint &second, const QPoint &toKnow) const
    {
        //zrata vzdialenosti medzi first a second, medzi first a toKnow a medzi toKnow a second
        //potom porovna s odchylkou 0.1 z kazdej strany
        float first_second = sqrt(std::pow((second.x()-first.x()),2)+std::pow((second.y()-first.y()),2));
        float first_point = sqrt(std::pow((toKnow.x()-first.x()),2)+std::pow((toKnow.y()-first.y()),2));
        float point_second = sqrt(std::pow((second.x()-toKnow.x()),2)+std::pow((second.y()-toKnow.y()),2));
        if((first_second  >= first_point + point_second - 0.1) && (first_second  <= first_point + point_second + 0.1))
            return true;
        return false;
    }

    //spoločná fcia, ktora vrati index hranice, na ktoru sme klikli
    int pointIsOnBorder(QPoint p) const {
        for (int i = 1; i < borders_.size(); i++)
        {
            if (abs(borders_[i].x() - p.x()) <= 5 &&
                abs(borders_[i].y() - p.y()) <= 5) {
                return i;
            }
        }
        return -1;
    }

    //ukladanie dat do streamu
    void store(QDataStream &out) {
        out << (quint32)tool_;
        out << borders_.size();
        for (auto point : borders_)
            out << point;
        out << color_;
        out << width_;
        out << rotation_;
        out << filled_;
    }

    //vyberanie dat zo streamu do objektu
    void load(QDataStream &in) {
        borders_.clear();
        size_t vectorSize;
        in >> vectorSize;
        for (size_t i = 0; i < vectorSize; i++) {
            QPoint tempPoint;
            in >> tempPoint;
            borders_.push_back(tempPoint);
        }
        in >> color_;
        in >> width_;
        in >> rotation_;
        in >> filled_;
        SetPosition(0,0);
    }
};

//objekt elipsy
class Circle : public Object
{
public:
    //definovany obdlznikom
    QRect ell;

    //kazda class ma vlastny konstruktor kvoli nastaveniu tool
    Circle() { tool_ = Tool::enCir; };
    Circle(Body borders, QColor color, int width, bool fill) {
        borders_ = borders;
        color_ = color;
        width_ = width;
        tool_ = Tool::enCir;
        filled_ = fill;
        SetPosition(0,0);
    };

    //funkcia ktora podla vzorca na elipsu prerata, ci sa bod p nachadza na jej hranici
    bool pointIsOver(QPoint p) const
    {
        float x = (borders_[0].x()+borders_[1].x())/2;
        float y = (borders_[0].y()+borders_[1].y())/2;
        float a = abs(x-borders_[1].x());
        float b = abs(y-borders_[1].y());
        float result = std::pow(p.x()-x,2)/std::pow(a,2) + std::pow(p.y()-y,2)/std::pow(b,2);
        if (result >= 0.95 && result <= 1.05)
            return true;
        return false;
    }

    void SetPosition(int xOff, int yOff)
    {
        if (xOff != 0 || yOff !=0 ) {
        //ku vsetkym bodom borders_ priratame offset
            for (size_t i = 0; i < borders_.size(); i++) {
                borders_[i].setX(borders_[i].x() + xOff);
                borders_[i].setY(borders_[i].y() + yOff);
            }
        }

        //upravime objekt na vykreslenie a stred
        ell.setRect(borders_[0].x(), borders_[0].y(), borders_[1].x() - borders_[0].x(), borders_[1].y() - borders_[0].y());
    }
};

class Polygon : public Object
{
public:
    QPolygon pol;

    Polygon() { tool_ = Tool::enTr; };
    Polygon(Body borders, QColor color, int width, bool fill) {
        borders_ = borders;
        color_ = color;
        width_ = width;
        tool_ = Tool::enTr;
        filled_ = fill;
        SetPosition(0,0);
    };

    //pri polygone skontrolujeme ci bod nie je nad jednou z ciar, ktore ten polygon definuju
    bool pointIsOver(QPoint p) const {
        for (int i = 0; i < borders_.size(); i++) {
            if (i == borders_.size() - 1) {
                if (isOnLine(borders_[i],borders_[0],p))
                    return true;
            }
            else {
                if (isOnLine(borders_[i],borders_[i+1],p))
                    return true;
            }
        }
        return false;
    }

    void SetPosition(int xOff, int yOff)
    {
        //vyprazdnime polygon
        pol.clear();
        //prerobime body
        if (xOff != 0 || yOff !=0 ) {
            for (size_t i = 0; i < borders_.size(); i++) {
                borders_[i].setX(borders_[i].x() + xOff);
                borders_[i].setY(borders_[i].y() + yOff);
            }
        }

        //natlacime naspet do pol
        for (int i = 0; i < borders_.size(); i++)
        {
            pol.push_back(borders_[i]);
        }
    }
};

class Rectangle : public Object
{
public:
    QRect rect;

    Rectangle() { tool_ = Tool::enRec; };
    Rectangle(Body borders, QColor color, int width, bool fill) {
        color_ = color;
        width_ = width;
        tool_ = Tool::enRec;
        borders_ = borders;
        filled_ = fill;
        SetPosition(0,0);
    };

    //prechadzame 4 usecky, ktore definuju obdlznik
    bool pointIsOver(QPoint p) const {
        if(isOnLine(QPoint(borders_[0].x(),borders_[0].y()),QPoint(borders_[0].x(),borders_[1].y()),p) ||
           isOnLine(QPoint(borders_[0].x(),borders_[1].y()),QPoint(borders_[1].x(),borders_[1].y()),p) ||
           isOnLine(QPoint(borders_[1].x(),borders_[1].y()),QPoint(borders_[1].x(),borders_[0].y()),p) ||
           isOnLine(QPoint(borders_[1].x(),borders_[0].y()),QPoint(borders_[0].x(),borders_[0].y()),p))
                return true;
        return false;
    }

    //rovnako ako pri elipse
    void SetPosition(int xOff, int yOff)
    {
        if (xOff != 0 || yOff !=0 ) {
            for (size_t i = 0; i < borders_.size(); i++) {
                borders_[i].setX(borders_[i].x() + xOff);
                borders_[i].setY(borders_[i].y() + yOff);
            }
        }

        rect.setRect(borders_[0].x(), borders_[0].y(), borders_[1].x() - borders_[0].x(), borders_[1].y() - borders_[0].y());
    }
};

class Line : public Object
{
public:
    QLine line;

    Line() { tool_ = Tool::enLi; };
    Line(Body borders, QColor color, int width) {
        borders_ = borders;
        color_ = color;
        width_ = width;
        tool_ = Tool::enLi;
        SetPosition(0,0);
    };

    bool pointIsOver(QPoint p) const {
        return isOnLine(borders_[0],borders_[1],p);
    }

    void SetPosition(int xOff, int yOff)
    {
        if (xOff != 0 || yOff !=0 ) {
            for (size_t i = 0; i < borders_.size(); i++) {
                borders_[i].setX(borders_[i].x() + xOff);
                borders_[i].setY(borders_[i].y() + yOff);
            }
        }

        line.setLine(borders_[0].x(),borders_[0].y(),borders_[1].x(),borders_[1].y());
        }
};

#endif // OBJECT_H
