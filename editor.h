#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QString>
#include <QMouseEvent>
#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QGroupBox>
#include <QBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QColorDialog>

//spaja ui s drawPlace
namespace Ui {
class editor;
}

class editor : public QMainWindow
{
    Q_OBJECT

public:
    explicit editor(QWidget *parent = 0);
    ~editor();

    //procedury ako sloty na posielanie signalov
private slots:
    void on_colorButton_clicked();
    void on_actionSave_triggered();
    void on_actionExport_triggered();
    void on_actionClean_triggered();
    void on_actionSet_Canvas_triggered();
    void on_closeButton_clicked();
    void on_okButton_clicked();
    bool is_digit(std::string s);
    void on_actionLoad_triggered();

private:
    Ui::editor *ui;

signals:
    void changeColor(QColor c);
};

#endif // EDITOR_H
