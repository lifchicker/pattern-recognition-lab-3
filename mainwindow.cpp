#include <QtGui>
#include "mainwindow.h"

#include <math.h>
#include <memory.h>
#include <fstream>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
    m(0), a(NULL), b(NULL), __a__(NULL), x(NULL), selectionSize(100),
    inputDataLoaded(false), kxy(0.0), middleX(0.0), middleY(0.0), shiftX(10.0), shiftY(10.0),
    sigmaX(0.0), sigmaY(0.0)
{
    ui.setupUi(this);

    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(generate()));
    connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(calculate_values()));
    //connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(draw()));
    connect(ui.zoomX, SIGNAL(sliderReleased()), this, SLOT(draw()));
    connect(ui.zoomY, SIGNAL(sliderReleased()), this, SLOT(draw()));
    connect(ui.component1, SIGNAL(editingFinished()), this, SLOT(calculate_values()));
    connect(ui.component2, SIGNAL(editingFinished()), this, SLOT(calculate_values()));

    srand(QTime::currentTime().elapsed());
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

void MainWindow::calculate_values()
{
    if (!inputDataLoaded)
        return;

    int vec1 = ui.component1->value();
    int vec2 = ui.component2->value();

    middleX = 0.0;
    middleY = 0.0;
    for (int i = 0; i < selectionSize; ++i)
    {
        middleX += x[i][vec1];
        middleY += x[i][vec2];
    }
    middleX /= static_cast<double>(selectionSize);
    middleY /= static_cast<double>(selectionSize);

    ui.labelMiddleX->setText(QString("%1").arg(middleX));
    ui.labelMiddleY->setText(QString("%1").arg(middleY));

    sigmaX = 0.0;
    sigmaY = 0.0;
    kxy = 0.0;
    for (int i = 0; i < selectionSize; ++i)
    {
        sigmaX += (x[i][vec1] - middleX)*(x[i][vec1] - middleX);
        sigmaY += (x[i][vec2] - middleY)*(x[i][vec2] - middleY);
        kxy += (x[i][vec1] - middleX)*(x[i][vec2] - middleY);
    }
    sigmaX /= selectionSize;
    sigmaY /= selectionSize;
    kxy /= selectionSize;

    sigmaX = sqrt(sigmaX);
    sigmaY = sqrt(sigmaY);

    ui.labelSigmaX->setText(QString("%1").arg(sigmaX));
    ui.labelSigmaY->setText(QString("%1").arg(sigmaY));
    ui.labelKxy->setText(QString("%1").arg(kxy));

    draw();
}

double MainWindow::calculate_y1(double x, double p)
{
    double r = kxy/(sigmaX*sigmaY);
    double sx2 = sigmaX*sigmaX;
    double sy2 = sigmaY*sigmaY;
    
    double lambdad = 2.0*(1.0 - r*r)*log(2.0*M_PI*sigmaX*sigmaY*sqrt(1.0 - r*r)*p);
    //if (lambdad < 0.0)
    //    lambdad = 0.0 - lambdad;

    return (r*sigmaY*x + sigmaX*middleY - r*sigmaY*middleX +
            sqrt(r*r*sy2*x*x - 2.0*r*r*sy2*x*middleX +
                 r*r*sy2*middleX*middleX - sy2*x*x +
                 2.0*sy2*x*middleX - sy2*middleX*middleX +
                 lambdad*sx2*sy2)
            )/sx2;
}

double MainWindow::calculate_y2(double x, double p)
{
    double r = kxy/(sigmaX*sigmaY);
    double sx2 = sigmaX*sigmaX;
    double sy2 = sigmaY*sigmaY;

    double lambdad = -2.0*(1.0 - r*r)*log(2.0*M_PI*sigmaX*sigmaY*sqrt(1.0 - r*r)*p);

    return (r*sigmaY*x + sigmaX*middleY - r*sigmaY*middleX -
            sqrt(r*r*sy2*x*x - 2.0*r*r*sy2*x*middleX +
                 r*r*sy2*middleX*middleX - sy2*x*x +
                 2.0*sy2*x*middleX - sy2*middleX*middleX +
                 lambdad*sx2*sy2)
            )/sx2;
}

double MainWindow::drand()
{
    return (double)rand()/RAND_MAX;
}

void MainWindow::draw()
{
    int vec1 = ui.component1->value();
    int vec2 = ui.component2->value();

    shiftX = 1.0;
    shiftY = 7.0;
    QPen pen;
    //pen.setBrush(QBrush(QColor(255, 0, 0, 255), Qt::SolidPattern));
    pen.setColor(QColor(255, 0, 0, 255));
    pen.setWidth(1);
    QGraphicsScene * scene = new QGraphicsScene(ui.graphicsView->rect(), this);

    scene->addLine(plot_x(0.0), plot_y(0.0), plot_x(1.0), plot_y(0.0), pen);
    scene->addLine(plot_x(0.0), plot_y(0.0), plot_x(0.0), plot_y(1.0), pen);

    scene->addLine(plot_x(middleX), plot_y(middleY), plot_x(middleX)+1.0, plot_y(middleY)+1.0, pen);

    for (int i = 0; i < selectionSize; ++i)
        scene->addLine(plot_x(x[i][vec1]), plot_y(x[i][vec2]), plot_x(x[i][vec1])+1.0, plot_y(x[i][vec2])+1.0, pen);

    QPainterPath path1;
    QPainterPath path2;
    QPainterPath path3;
    draw_ellipse(path1, 0.2);
    scene->addPath(path1, pen);
    draw_ellipse(path2, 0.6);
    scene->addPath(path2, pen);
    draw_ellipse(path3, 1.0);
    scene->addPath(path3, pen);

    scene->setBackgroundBrush(QBrush(QColor(0, 255, 0, 255)));
    ui.graphicsView->setScene(scene);
    ui.graphicsView->show();
}

void MainWindow::draw_ellipse(QPainterPath &path, double p)
{
    double y = plot_y(calculate_y1(0.0, p));
    path.moveTo(plot_x(0.0), y);

    for (int i = 0; i <= 100; ++i)
    {
        double currentx = 100.0/static_cast<double>(i);
        path.lineTo(plot_x(currentx), plot_y(calculate_y1(currentx, p)));
    }

    for (int i = 100; i >= 0; --i)
    {
        double currentx = 100.0/static_cast<double>(i);
        path.lineTo(plot_x(currentx), plot_y(calculate_y2(currentx, p)));
    }
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

    for (int i = 0; i < selectionSize; ++i)
    {
        x[i] = new double[m];
        generate_vector(x[i]);
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
    for (int i = 0; i < m; ++i)
    {
        __a__[i] = new double[m];
        memset(__a__[i], 0, sizeof(double)*m);
    }

    for (int i = 0; i < m; ++i)
        for (int j = 0; j <= i; ++j)
        {
            double sum1 = 0.0;
            for (int k = 0; k < j; ++k)
                sum1 += __a__[i][k]*__a__[j][k];

            double sum2 = 0.0;
            for (int k = 0; k < j; ++k)
                sum2 += __a__[j][k]*__a__[j][k];

            if ((b[j][j] - sum2) <= 0.0)
            {
                QMessageBox::critical(this, tr("Generation ||A|| failed"),
                                      tr("Invalid matrix of correlations!"),
                                      QMessageBox::Ok);
                delete[] __a__;
                __a__ = NULL;
                return;
            }

            __a__[i][j] = (b[i][j] - sum1)/sqrt(b[j][j] - sum2);
        }
}

// generate random verctor with normal distribution
void MainWindow::generate_normal_vector(double * vec)
{
    if (!vec)
        return;

    for (int i = 0; i < m; ++i)
        vec[i] = drand();
}

// generate random vector with predefined partition law
void MainWindow::generate_vector(double * vec)
{
    if (!vec)
        return;

    double * nv = new double[m];
    generate_normal_vector(nv);

    for (int i = 0; i < m; ++i)
    {
        double stringSum = 0.0;
        for (int j = 0; j < m; ++j)
            stringSum += __a__[i][j]*nv[j];
        vec[i] = stringSum + a[i];
    }

    delete[] nv;

    std::fstream file;
    file.open("./log.txt", std::ios_base::out | std::ios_base::app );
    for (int i = 0; i < m; i++)
        file << nv[i] << " ";
    file << std::endl;
    file.close();
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
    for (int i = 0; i < m; ++i)
        in >> a[i];

    //create matrix of collerations and fill them by values from file
    b = new double*[m];
    for (int i = 0; i < m; ++i)
    {
        b[i] = new double[m];
        for (int j = 0; j < m; ++j)
            in >> b[i][j];
    }

    in.close();

    generate__a__();

    inputDataLoaded = true;
}

double MainWindow::plot_x(double x)
{
    if (ui.zoomX->value() > 70)
        return x*((static_cast<double>(ui.zoomX->value()) - 65.0)/5.0)*(ui.graphicsView->width() - shiftX) + shiftX;  //zoom in
    else
        return x*(static_cast<double>(ui.zoomX->value())/70.0)*(ui.graphicsView->width() - shiftX) + shiftX;    //zoom out
}

double MainWindow::plot_y(double y)
{
    if (ui.zoomY->value() > 70)
        return static_cast<double>(ui.graphicsView->height()) - y*((static_cast<double>(ui.zoomY->value()) - 65.0)/5.0)*(ui.graphicsView->height() - shiftY) - shiftY;
    else
        return static_cast<double>(ui.graphicsView->height()) - y*(static_cast<double>(ui.zoomY->value())/70.0)*(ui.graphicsView->height() - shiftY) - shiftY;
}
