#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class QPainterPath;
class QRectF;
class QGraphicsScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~MainWindow();

private slots:
    void calculate_values();
    void draw();
    void generate();
    void load();

private:
    void calculate_bounding_rect(int vec1, int vec2);
    double drand();
    void draw_ellipse(QPainterPath &path, double p);
    void generate__a__();
    void generate_normal_vector(double * vec);
    void generate_vector(double * vec);

    double plot_x(double x);
    double plot_y(double y);

    double calculate_y1(double x, double p);
    double calculate_y2(double x, double p);

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
    double kxy;
    double middleX;
    double middleY;
    double shiftX;
    double shiftY;
    double sigmaX;
    double sigmaY;
    double r;

    QRectF boundingRect;
    QGraphicsScene * scene;
};

#endif // MAINWINDOW_H
