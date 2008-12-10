#include <QtGui>
#include "mainwindow.h"

#include <math.h>
#include <memory.h>
#include <fstream>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
    m(0), a(NULL), b(NULL), __a__(NULL), x(NULL), selectionSize(100),
    inputDataLoaded(false), shiftx(0.0), shifty(0.0)
{
    ui.setupUi(this);

    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(generate()));
    //connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(calculateValues()));
    connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(draw()));
    connect(ui.zoom, SIGNAL(sliderReleased()), this, SLOT(draw()));
    connect(ui.component1, SIGNAL(editingFinished()), this, SLOT(calculateValues()));
}

MainWindow::~MainWindow()
{
    if (a != NULL)
    {
        delete[] a;
        a = NULL;
    }

    if (b != NULL)
    {
        delete[] b;
        b = NULL;
    }

    if (__a__ != NULL)
    {
        delete __a__;
        __a__ = NULL;
    }

    if (x != NULL)
    {
        delete x;
        x = NULL;
    }
}

void MainWindow::calculateValues()
{
    if (!inputDataLoaded)
        return;

    int vec1 = ui.component1->value();
    int vec2 = ui.component2->value();

    double middlex = 0.0;
    double middley = 0.0;
    for (int i = 0; i < selectionSize; i++)
    {
        middlex += x[i][vec1];
        middley += x[i][vec2];
    }
    middlex /= selectionSize;
    middley /= selectionSize;

    ui.labelMiddleX->setText(QString("%1").arg(middlex));
    ui.labelMiddleY->setText(QString("%1").arg(middley));


    double sigmax = 0.0;
    double sigmay = 0.0;
    double kxy = 0.0;
    for (int i = 0; i < selectionSize; i++)
    {
        sigmax += (x[i][vec1] - middlex)*(x[i][vec1] - middlex);
        sigmay += (x[i][vec2] - middley)*(x[i][vec2] - middley);
        kxy += (x[i][vec1] - middlex)*(x[i][vec2] - middley);
    }
    sigmax /= selectionSize;
    sigmay /= selectionSize;
    kxy /= selectionSize;

    sigmax = sqrt(sigmax);
    sigmay = sqrt(sigmay);

    ui.labelSigmaX->setText(QString("%1").arg(sigmax));
    ui.labelSigmaY->setText(QString("%1").arg(sigmay));
    ui.labelKxy->setText(QString("%1").arg(kxy));

    draw();
}

double MainWindow::drand()
{
    return (double)rand()/RAND_MAX;
}

void MainWindow::draw()
{
    QPen pen;
    //pen.setBrush(QBrush(QColor(255, 0, 0, 255), Qt::SolidPattern));
    pen.setColor(QColor(255, 0, 0, 255));
    pen.setWidth(10);
    QGraphicsScene * scene = new QGraphicsScene(ui.graphicsView->rect(), this);

    scene->addLine(plotX(0.0), plotY(0.0), plotX(100.0), plotY(100.0), pen);
    scene->addLine(0.0, 0.0, 100.0, 100.0, pen);

/*
        for (int i = 0; i < points.size()-1; i++)
        {
                scene->addLine(points[i]._x*780.0+10.0, 290.0-points[i]._y*280.0, points[i+1]._x*780.0+10.0, 290.0-points[i+1]._y*280.0, pen);
        }
        scene->setBackgroundBrush(QBrush(QColor(0, 255, 0, 255)));

        return scene;
*/

    scene->setBackgroundBrush(QBrush(QColor(0, 255, 0, 255)));
    ui.graphicsView->setScene(scene);
    ui.graphicsView->show();
}

void MainWindow::generate()
{
    if (!inputDataLoaded)
        return;

    selectionSize = ui.selectionDimention->value();

    if (x != NULL)
    {
        delete[] x;
        x = NULL;
    }

    x = new double*[selectionSize];

    for (int i = 0; i < selectionSize; i++)
    {
        x[i] = new double[m];
        generateVector(x[i]);
    }

    ui.component1->setMaximum(ui.selectionDimention->value());
    ui.component2->setMaximum(ui.selectionDimention->value());
}

void MainWindow::generate__a__()
{
    if (__a__ != 0)
    {
        delete __a__;
        __a__ = NULL;
    }


//may be need refactor
    __a__ = new double*[m];
    for (int i = 0; i < m; i++)
    {
        __a__[i] = new double[m];
        memset(__a__[i], 0, sizeof(double)*m);
    }

    for (int i = 0; i < m; i++)
        for (int j = 0; j <= i; j++)
        {
            double sum1 = 0.0;
            for (int k = 0; k < j; k++)
                sum1 += __a__[i][k]*__a__[j][k];

            double sum2 = 0.0;
            for (int k = 0; k < j; k++)
                sum2 += __a__[j][k]*__a__[j][k];

            __a__[i][j] = (b[i][j] - sum1)/sqrt(b[j][j] - sum2);
        }
}

// generate random verctor with normal distribution
void MainWindow::generateNormalVector(double * vec)
{
    if (!vec)
        return;

    srand(QTime::currentTime().elapsed());
    for (int i = 0; i < m; i++)
        vec[i] = drand();
}

// generate random vector with predefined partition law
void MainWindow::generateVector(double * vec)
{
    if (!vec)
        return;

    double * nv = new double[m];
    generateNormalVector(nv);

    for (int i = 0; i < m; i++)
    {
        double stringSum = 0.0;
        for (int j = 0; j < m; j++)
            stringSum += __a__[i][j]*nv[j];
        vec[i] = stringSum + a[i];
    }

    delete[] nv;
}

void MainWindow::load()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select the input file"));

    if (filename.isEmpty())
        return;

    //open file
/*
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen() || !file.isReadable())
        return;

    QDataStream in(&file);
*/
    std::fstream in;
    in.open(filename.toAscii().data(), std::ios_base::in);
    if (!in.is_open())
        return;

    //read dimention of X
    in >> m;

    //create vector of average values and fill them by values from file
    a = new double[m];
    for (int i = 0; i < m; i++)
        in >> a[i];

    //create matrix of collerations and fill them by values from file
    b = new double*[m];
    for (int i = 0; i < m; i++)
    {
        b[i] = new double[m];
        for (int j = 0; j < m; j++)
            in >> b[i][j];
    }

    in.close();

    generate__a__();

    inputDataLoaded = true;
}

double MainWindow::plotX(double x)
{
    if (ui.zoom->value() > 70)
        return x*((static_cast<double>(ui.zoom->value()) - 65.0)/5.0) + shiftx;  //zoom in
    else
        return x*(static_cast<double>(ui.zoom->value())/70.0) + shiftx;    //zoom out
}

double MainWindow::plotY(double y)
{
    if (ui.zoom->value() > 70)
        return static_cast<double>(ui.graphicsView->height()) - y*((static_cast<double>(ui.zoom->value()) - 65.0)/5.0) - shifty;
    else
        return static_cast<double>(ui.graphicsView->height()) - y*(static_cast<double>(ui.zoom->value())/70.0) - shifty;
}
