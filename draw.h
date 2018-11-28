#ifndef DRAW_H
#define DRAW_H

#include "object.h"
#include <QString>
#include <QMouseEvent>
#include <QWidget>
#include <QPainter>
#include <vector>
#include <memory>
#include <QTransform>

//pouzivam unique_ptr na ulozenie objektov
typedef std::unique_ptr<Object> ObjectPtr;

//pomocny struct, ktory urcuje, ci sa zmenila velkost vektora
struct VectorSizeChanged
{
    size_t currentSize = 0;
    bool changed(size_t size);
};

//draw je vykreslovacia plocha
class draw : public QWidget
{
    Q_OBJECT
public:
    explicit draw(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    //bod kde sa aktualne nachadza mys, ked je stlacene lave tlacitko
    QPoint point;
    //bod kde bolo stlacene lave tlacitko
    QPoint beginPoint;
    //aktualne vlastnosti objektu
    Tool tool;
    QColor color;
    int width;
    Body borders;
    //ci je stlacene lave tlacitko
    bool keyHold;
    //vytvaranie objektu sa skoncilo, uloz ho
    bool drawSave;
    //aktualne pero
    QPen currentPen;
    //pero vyberu
    QPen selectPen;
    //pero vykreslovaneho objektu
    QPen objectPen;
    //vektor kde mame ulozene vsetky doteraz vytvorene objekty
    std::vector<ObjectPtr> toDraw;
    //pomocny bool na tvorbu polygonov
    bool polyDraw;
    VectorSizeChanged vsc;
    //pomocny vektor do ktoreho ukladame docasne ciary polygonu, ked ho vytvarame
    Body tempLine;
    //index aktualne vybraneho objektu
    size_t currentlySelected;
    //index objektu, na ktory sme klikli
    size_t clickedObj;
    //ci sa ma objekt vyplnit
    bool fillObject;
    //index aktualnej hranice
    int onBorderIndex;
    //nastavi niektore hodnoty draw
    void setToDefault();
    //fcia ktora vracia id objektu na ktory sme klikli
    std::pair<int, int> isMouseOverObject(QPoint p) const;

signals:

public slots:
    //sloty v draw, prijimaju signaly cez connect od editoru na upravu hodnot v draw
    //komunikacia medzi UI a vykreslovacou plochou
    void shapeChangedtoLine();
    void shapeChangedtoRectangle();
    void shapeChangedtoCircle();
    void shapeChangedtoPolygon();
    void pickModeOn();
    void newWidth(int w);
    void newColor(QColor c);
    void rotate(int r);
    void fill(bool f);
};

#endif // DRAW_H
