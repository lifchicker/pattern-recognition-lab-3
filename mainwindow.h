#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~MainWindow();

private slots:
    void calculateValues();
    void draw();
    void generate();
    void load();

private:
    double drand();
    void generate__a__();
    void generateNormalVector(double * vec);
    void generateVector(double * vec);

    double plotX(double x);
    double plotY(double y);

    //input data (from file or consol)
    int m;  //dimention of X
    double * a; //vercor of average values
    double ** b;    //matrix of correlations

    //generated data
    double ** __a__;    // this is the ||A|| matrix
    double ** x;        //selection
    int selectionSize;
    bool inputDataLoaded;

    Ui::MainWindowClass ui;
    double shiftx;
    double shifty;
};

#endif // MAINWINDOW_H
