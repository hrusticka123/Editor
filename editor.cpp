#include "editor.h"
#include "ui_editor.h"
#include "draw.h"

editor::editor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::editor)
{
    ui->setupUi(this);

    //jednotlive connecty, pri zmene stavu objektu na ui editor posle hodnotu do drawPlace
    connect(ui->lineButton, SIGNAL(clicked()), ui->drawPlace, SLOT(shapeChangedtoLine()));
    connect(ui->rectangleButton, SIGNAL(clicked()), ui->drawPlace, SLOT(shapeChangedtoRectangle()));
    connect(ui->circleButton, SIGNAL(clicked()), ui->drawPlace, SLOT(shapeChangedtoCircle()));
    connect(ui->polygonButton, SIGNAL(clicked()), ui->drawPlace, SLOT(shapeChangedtoPolygon()));
    connect(ui->pickButton, SIGNAL(clicked()), ui->drawPlace, SLOT(pickModeOn()));
    connect(ui->sizeSlider, SIGNAL(sliderMoved(int)), ui->drawPlace, SLOT(newWidth(int)));
    connect(this, SIGNAL(changeColor(QColor)), ui->drawPlace, SLOT(newColor(QColor)));
    connect(ui->rotateSlider, SIGNAL(sliderMoved(int)),ui->drawPlace, SLOT(rotate(int)));
    connect(ui->fillBox, SIGNAL(toggled(bool)),ui->drawPlace, SLOT(fill(bool)));

    //prvok ktory upravuje velkost canvasu je na zaciatku neviditelny
    ui->canvasSize->setVisible(false);
}

editor::~editor()
{
    delete ui;
}

//dorobeny ColorDialog na vyber farieb
//na zmenu sa vola osobitny changeColor signal
void editor::on_colorButton_clicked()
{
    QColor newColor = QColorDialog::getColor();
    emit changeColor(newColor);
}
//export do obrazkovych formatov
void editor::on_actionExport_triggered()
{
    QString imagePath = QFileDialog::getSaveFileName(this,tr("Export Image as"),"", tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" ));
    ui->drawPlace->grab().save(imagePath);
}

//ulozenie do upravovatelneho .edf filu
void editor::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),"", tr("Editor File (*.edf);;All Files (*)" ));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Nepodarilo sa otvoriť súbor"),
            file.errorString());
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);
    out << ui->drawPlace->toDraw.size();
    for (auto && x : ui->drawPlace->toDraw) {
        x->store(out);
    }
    file.close();
}

//zase otvorenie .edf filu
void editor::on_actionLoad_triggered()
{
    //uvodny dialog
    Tool t;
    size_t count;
    ObjectPtr objekt;
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"), "",tr("Editor File (*.edf);;All Files (*)"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
            file.errorString());
        return;
    }
    //stream in na otvoreny file
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);
    //vycisti sa toDraw
    ui->drawPlace->toDraw.clear();
    //vlozime udaj o pocte objektov
    in >> count;
    //nasypeme udaje objektoch, vytvorime objekty a ulozime do toDraw
    for(size_t i = 0; i < count; i++) {
        in >> t;
        switch (t) {
        case Tool::enRec:
        {
            Rectangle r;
            r.load(in);
            objekt = std::make_unique<Rectangle>(r);
            ui->drawPlace->toDraw.push_back(std::move(objekt));
            break;
        }
        case Tool::enTr:
        {
            Polygon p;
            p.load(in);
            objekt = std::make_unique<Polygon>(p);
            ui->drawPlace->toDraw.push_back(std::move(objekt));
            break;
        }
        case Tool::enCir:
        {
            Circle c;
            c.load(in);
            objekt = std::make_unique<Circle>(c);
            ui->drawPlace->toDraw.push_back(std::move(objekt));
            break;
        }
        case Tool::enLi:
        {
            Line l;
            l.load(in);
            objekt = std::make_unique<Line>(l);
            ui->drawPlace->toDraw.push_back(std::move(objekt));
            break;
        }
        }
    }
    file.close();
    repaint();
}

//premazanie canvasu
void editor::on_actionClean_triggered()
{
    ui->drawPlace->toDraw.clear();
}

//ukazanie okna na zmenu velkosti canvasu
void editor::on_actionSet_Canvas_triggered()
{
    ui->canvasSize->setVisible(true);
}

//zase zrusenie na tlacidlo Close
void editor::on_closeButton_clicked()
{
    ui->canvasSize->setVisible(false);
}

//nastavenie canvasu na tlacidlo OK
void editor::on_okButton_clicked()
{
    std::string width = ui->widthEdit->text().toStdString();
    std::string height = ui->heightEdit->text().toStdString();

    //kontrola ci boli napisane cisla
    if(!is_digit(width) || !is_digit(height)) {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Width or height is not a number");
        messageBox.setFixedSize(500,200);
        return;
    }

    ui->drawPlace->resize(std::stoi(width),std::stoi(height));
    ui->canvasSize->setVisible(false);
}

bool editor::is_digit(std::string s)
{
    return std::all_of(s.begin(), s.end(), ::isdigit);
}
