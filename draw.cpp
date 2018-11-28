#include "draw.h"

//vrati dvojicu, index objektu v toDraw a index hranice (ak sme neklikli na hranicu ale ciaru, vrati -1)
std::pair<int, int> draw::isMouseOverObject(QPoint p) const
{
    //inverzna transformacia
    QTransform backTrans;
    //prechadzame vsetky objekty toDraw
    int i = 0;
    for (auto && x : toDraw)
    {
        //nastavime transformaciu podla ktorej bol upraveny objekt
        QPoint rotationCenter = x->GetBorders()[0];
        backTrans.translate( rotationCenter.x(),  rotationCenter.y());
        backTrans.rotate(x->GetRotation());
        backTrans.translate(-rotationCenter.x(), -rotationCenter.y());
        //vytvorime novy bod newp na ktory namapujeme inverznu transformaciu
        QPoint newp = backTrans.inverted().map(p);
        backTrans.reset();
        //volame pointIsOver objektu na newp
        int borderIndex = x->pointIsOnBorder(newp);
        //vracia ked sme klikli na jeden z hranicnych bodov
        if (borderIndex != -1)
            return std::make_pair(i, borderIndex);
        //klikli sme na ciaru ktora vykresluje objekt
        else if (x->pointIsOver(newp))
            return std::make_pair(i, -1);
        i++;
    }
    //klikli sme vedla
    return std::make_pair(-1,-1);
}

draw::draw(QWidget *parent) : QWidget(parent)
{
    //defaultne hodnoty
    keyHold = false;
    drawSave = false;
    tool = Tool::enNone;
    color = Qt::red;
    width = 5;
    polyDraw = false;
    currentlySelected = -1;
    fillObject = false;
}

void draw::setToDefault()
{
    //vypnutie polydraw, vycistenie hranic a zrusenie picku, ak nejaky je
    polyDraw = false;
    borders.clear();
    if (currentlySelected != -1) {
        toDraw.at(currentlySelected)->Select(false);
    }
    currentlySelected = -1;
}

//fcia VectorSizeChanged na overenie, ci doslo k zmene
bool VectorSizeChanged::changed(size_t size){
        if (size > currentSize)
        {
            currentSize  = size;
            return true;
        }
        else
            return false;

}

//stlacenie mysi
void draw::mousePressEvent(QMouseEvent *e)
{
    //bolo stlacene lave tlacitko
    if (e->button() == Qt::LeftButton)
    {
        //pokial ide o vyber objektu
        if (tool == Tool::enPick)
        {
            //informacie o vybranom objekte
            auto clickedObjectInfo = isMouseOverObject(e->pos());
            clickedObj = clickedObjectInfo.first;
            onBorderIndex = clickedObjectInfo.second;

            //ak sme klikli na nejaky objekt (isMouseOverObject vracia -1 na first, ak ziadny nenasiel)
            if (clickedObj != -1)
            {
                //ulozime si miesto kliknutia do beginPoint
                beginPoint = e->pos();
                //zrusime aktualne vybrany objekt, ak nejaky taky je
                if (currentlySelected != -1) {
                    toDraw.at(currentlySelected)->Select(false);
                }
                //nastav teraz kliknuty na vybrany
                toDraw.at(clickedObj)->Select(true);
                currentlySelected = clickedObj;
            }
            //ak sme klikli mimo akehokolvek objektu, proste zrus aktualne vybrany
            else {
                if (currentlySelected != -1)
                    toDraw.at(currentlySelected)->Select(false);
                currentlySelected = -1;
            }
        }
        //pokial ide o vykreslenie noveho objektu
        else
        {
            //zase ulozime do beginPoint
            beginPoint = e->pos();
            //pripad ze ide o zaciatok kreslenia polygonu
            if (tool == Tool::enTr && !polyDraw)
            {
                //ulozime si prvy bod do vektora, ktory bude polygon definovat
                //nastavime zaciatok vykreslovania
                //velkost pomocneho structu bude 1
                polyDraw = true;
                borders.push_back(beginPoint);
                vsc.currentSize = 1;
            }
        }
        //na press laveho je tlacidlo stlacene
        keyHold = true;
    }
    //stlacenie praveho tlacidla premaze objekt na ktory sme nim klikli
    else if(e->button() == Qt::RightButton)
    {
        auto clickedObjectInfo = isMouseOverObject(e->pos());
        clickedObj = clickedObjectInfo.first;
        if (clickedObj != -1) {
            if (toDraw.at(clickedObj)->GetSelect())
                currentlySelected = -1;
            toDraw.erase(toDraw.begin() + clickedObj);
        }
    }
    update();
}

//pri pohybe mysi si ukladame do jej aktualnu poziciu do point
void draw::mouseMoveEvent(QMouseEvent *e)
{
    if (e->type() == QEvent::MouseMove)
    {
        point = e->pos();
        //rozne kurzory na rozne casti canvasu/objektu
        if (tool == Tool::enPick) {
            auto clickedObjectInfo = isMouseOverObject(point);
            if (clickedObjectInfo.second != -1)
                setCursor(Qt::SizeAllCursor);
            else if(clickedObjectInfo.first != -1 && clickedObjectInfo.second == -1 && keyHold)
                setCursor(Qt::ClosedHandCursor);
            else if(clickedObjectInfo.first != -1 && clickedObjectInfo.second == -1)
                setCursor(Qt::OpenHandCursor);
            else
                setCursor(Qt::ArrowCursor);
        }
    }
    update();
}

//pustenie mysi nam vypina keyHold
void draw::mouseReleaseEvent(QMouseEvent *event)
{
    //ak ide o polyDraw, chceme kazde pustenie mysi ulozit ako novy bod polygonu
    if (polyDraw)
    {
        //podmienka ktora overi, ci je posledny bod polygonu v 10 pixelovom radiu okolo prveho
        //ak ano, zrusime polyDraw a polygon mozeme ulozit
        if (event->pos().x()<=borders[0].x() + 10 &&
            event->pos().x()>=borders[0].x() - 10 &&
            event->pos().y()<=borders[0].y() + 10 &&
            event->pos().y()>=borders[0].y() - 10)
            polyDraw = false;
        //ak nie, pridame bod
        else
            borders.push_back(event->pos());
    }

    keyHold = false;
    update();
}

void draw::paintEvent(QPaintEvent *)
{
    //inicializacia painteru aj s antialiasingom
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);

    //pokial drzime tlacidlo, chceme simulovat vytvaranie objektu na pohyb mysi
    if (keyHold)
    {
        //nastavime pero podla aktualnych vlastnosti draw
        currentPen.setColor(color);
        currentPen.setCapStyle(Qt::RoundCap);
        currentPen.setWidth(width);
        //ak sa ma objekt vyplnit, nastavi sa brush
        if (fillObject)
            painter.setBrush(QBrush(color));
        else
            painter.setBrush(Qt::NoBrush);
        painter.setPen(currentPen);

        //aktualne tool, ktoreho tvorbu chceme simulovat
        switch(tool)
        {
        //ciara medzi bodom kde sme klikli a aktualnou poziciou mysi, kde tlacidlo stale drzime
        case Tool::enLi:
            painter.drawLine(beginPoint, point);
            break;
        case Tool::enRec:
            painter.drawRect(beginPoint.x(), beginPoint.y(), point.x() - beginPoint.x(), point.y() - beginPoint.y());
            break;
        case Tool::enCir:
            painter.drawEllipse(beginPoint.x(), beginPoint.y(), point.x() - beginPoint.x(), point.y() - beginPoint.y());
            break;
        //pri polygone staci ked simulujeme tvorbu poslednej tvorenej ciary
        case Tool::enTr:
            {
                painter.drawLine(borders.back(), point);
                break;
            }
        //pri vybere chceme posuvat cely objekt alebo menit velkost
        //rozdiel medzi kliknutym a pociatocnym nastavime ako offset, podla ktoreho zmenime poziciu celeho objektu
        case Tool::enPick:
        {
            //ci bol nejaky vybrany
            if (currentlySelected != -1) {
                //nebola vybrana hranica
                if (onBorderIndex == -1) {
                    toDraw.at(currentlySelected)->SetPosition(point.x()-beginPoint.x(),
                                                              point.y()-beginPoint.y());
                    beginPoint = point;
                }
                else {
                    //bola vybrana, musime zase nastavit spatnu transformaciu a namapovat bod kliknutia
                    QTransform backTrans;
                    QPoint rotationCenter = toDraw.at(currentlySelected)->GetBorders()[0];
                    backTrans.translate(rotationCenter.x(), rotationCenter.y());
                    backTrans.rotate(toDraw.at(currentlySelected)->GetRotation());
                    backTrans.translate(-rotationCenter.x(), -rotationCenter.y());
                    //vytvorime novy bod newp na ktory namapujeme inverznu transformaciu
                    QPoint newp = backTrans.inverted().map(point);
                    backTrans.reset();
                    toDraw.at(currentlySelected)->SetBorder(newp, onBorderIndex);
                }
            }
            break;
        }
        }
        //na zaver chceme, aby sa nam objekty ulozili
        drawSave = true;
    }
    //pokial chceme objekty ukladat
    else if (drawSave)
    {
        switch (tool) {
        case Tool::enTr:
        {
            //ak uz sme skoncili kreslenie polygonu
            if (!polyDraw) {
                //vybereme vsetky ciary z toDraw, ktore sme don vlozili, aby sme mohli polygon vytvorit
                for (int i=0; i < borders.size() - 1; i++)
                {
                    toDraw.pop_back();
                }

                //vytvarame novy Objekt, vlozime do toDraw na buduce vykreslenie, vycistime borders
                ObjectPtr Objekt (new Polygon(borders,color,width,fillObject));
                toDraw.push_back(std::move(Objekt));
                borders.clear();
                vsc.currentSize = 0;
            }
            //pokial sa polygon stale vykresluje
            else {
                //len ak sa nam zmenila velkost vector, teda bol pridany do nej novy bod
                if (vsc.changed(borders.size())) {
                    //tempLine su posledne 2 body z borders, predstavuju novo pridanu ciaru polygonu
                    tempLine.push_back(borders.end()[-1]);
                    tempLine.push_back(borders.end()[-2]);
                    ObjectPtr Objekt (new Line(tempLine,color,width));
                    toDraw.push_back(std::move(Objekt));
                    tempLine.clear();
                }
            }
            break;
        }
        //ostatne funguju podobne
        case Tool::enCir:
        {
            borders.push_back(beginPoint);
            borders.push_back(point);
            ObjectPtr Objekt (new Circle(borders,color,width,fillObject));
            toDraw.push_back(std::move(Objekt));
            borders.clear();
            break;
        }
        case Tool::enLi:
        {
            borders.push_back(beginPoint);
            borders.push_back(point);
            ObjectPtr Objekt (new Line(borders,color,width));
            toDraw.push_back(std::move(Objekt));
            borders.clear();
            break;
        }
        case Tool::enRec:
        {
            borders.push_back(beginPoint);
            borders.push_back(point);
            ObjectPtr Objekt (new Rectangle(borders,color,width,fillObject));
            toDraw.push_back(std::move(Objekt));
            borders.clear();
            break;
        }
        }
        //nakoiec vypneme ukladanie
        drawSave = false;
    }

    //vsetky objekty toDraw chceme vykreslit
    for (auto && x : toDraw) {
        //nastavime select pero
        selectPen.setColor(Qt::yellow);
        selectPen.setWidth(x->GetWidth() + 2);

        //object pero
        objectPen.setColor(x->GetColor());
        objectPen.setWidth(x->GetWidth());
        painter.setPen(objectPen);

        //stred rotacie
        QPoint rotationCenter = x->GetBorders()[0];

        //vytvorime potrebnu transformaciu pre dany objekt
        //najprv ju celu posunieme do stredu naseho objektu
        //potom mozeme objekt rotovat a scalovat
        //nasledne musime posunut pred vykreslenim objektu transformaciu naspet
        QTransform transform;
        transform.translate(rotationCenter.x(), rotationCenter.y());
        transform.rotate(x->GetRotation());
        transform.translate(-rotationCenter.x(), -rotationCenter.y());

        //podla toho ci je dany objekt vybrany alebo nie urcime ktore potrebujeme
        if(x->GetSelect()) {
            painter.setPen(x->GetColor());
            painter.drawEllipse(rotationCenter.x()-10, rotationCenter.y()-10,20,20);
            painter.setPen(selectPen);
            for (size_t i = 1; i < x->GetBorders().size(); i++) {
                painter.drawRect(transform.map(x->GetBorders()[i]).x() - 5, transform.map(x->GetBorders()[i]).y() - 5, 10, 10);
            }
        }
        else
            painter.setPen(objectPen);

        if (x->GetFilled())
            painter.setBrush(QBrush(x->GetColor()));
        else
            painter.setBrush(Qt::NoBrush);

        switch(x->GetTool())
        {
        //v kazdom pripade tool
        //najprv vytvorime pointer na konkretny objekt
        //nastavime transformaciu
        //vykreslime ho
        //zrusime tranformaciu
        case Tool::enLi:
        {          
            auto y = dynamic_cast<Line*>(x.get());
            painter.setTransform(transform);
            painter.drawLine(y->line);
            painter.resetTransform();
            break;
        }
        case Tool::enRec:
        {
            auto y = dynamic_cast<Rectangle*>(x.get());
            painter.setTransform(transform);
            painter.drawRect(y->rect);
            painter.resetTransform();
            break;
        }
        case Tool::enCir:
        {
            auto y = dynamic_cast<Circle*>(x.get());
            painter.setTransform(transform);
            painter.drawEllipse(y->ell);
            painter.resetTransform();
            break;
        }
        case Tool::enTr:
        {
            auto y = dynamic_cast<Polygon*>(x.get());
            painter.setTransform(transform);
            painter.drawPolygon(y->pol);
            painter.resetTransform();
            break;
        }
        }
    }
    painter.end();
}

//funkcie na komunikaciu medzi editorom a draw
//nastavia aktualny tool
void draw::shapeChangedtoLine()
{
    tool = Tool::enLi;
    setToDefault();
}
void draw::shapeChangedtoRectangle()
{
    tool = Tool::enRec;
    setToDefault();
}

void draw::shapeChangedtoCircle()
{
    tool = Tool::enCir;
    setToDefault();
}

void draw::shapeChangedtoPolygon()
{
    tool = Tool::enTr;
    setToDefault();
}

//nastavia novu sirku/farbu
//ak je nejaky objekt vybranym, zmenia jeho sirku/farbu
void draw::newWidth(int w)
{
    width = w;
    if (currentlySelected != -1 && tool == Tool::enPick)
        toDraw.at(currentlySelected)->SetWidth(width);
    repaint();
}

void draw::newColor(QColor c)
{
    color = c;
    if (currentlySelected != -1 && tool == Tool::enPick)
        toDraw.at(currentlySelected)->SetColor(color);
    repaint();
}

//spustenie vyberu
void draw::pickModeOn()
{
    tool = Tool::enPick;
    setToDefault();
}

//nastavenie rotacie pre vybrany objekt
void draw::rotate(int r) {
    if (currentlySelected != -1)
        toDraw.at(currentlySelected)->SetRotation((qreal)r);
    repaint();
}

//vypln
void draw::fill(bool f) {
    fillObject = f;
}
