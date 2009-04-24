#include <QtGui>
#include "mainwindow.h"

#include <math.h>
#include <memory.h>
#include <fstream>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
    m(0), a(NULL), b(NULL), __a__(NULL), x(NULL), selectionSize(100),
    inputDataLoaded(false), selectionGenerated(false), kxy(0.0), middleX(0.0), middleY(0.0),
    shiftX(10.0), shiftY(10.0), sigmaX(0.0), sigmaY(0.0), r(0.0), scene(NULL)
{
    ui.setupUi(this);

    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(generate()));
    connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(calculate_values()));
    //connect(ui.drawButton, SIGNAL(clicked()), this, SLOT(draw()));
    connect(ui.zoomX, SIGNAL(valueChanged(int)), this, SLOT(draw()));
    connect(ui.zoomY, SIGNAL(valueChanged(int)), this, SLOT(draw()));
    connect(ui.checkBoxSelection, SIGNAL(stateChanged(int)), this, SLOT(draw()));
    connect(ui.checkBoxMiddle, SIGNAL(stateChanged(int)), this, SLOT(draw()));
    connect(ui.checkBoxIsolines, SIGNAL(stateChanged(int)), this, SLOT(draw()));
    connect(ui.component1, SIGNAL(editingFinished()), this, SLOT(calculate_values()));
    connect(ui.component2, SIGNAL(editingFinished()), this, SLOT(calculate_values()));
    connect(ui.buttonSaveSelection, SIGNAL(clicked()), this, SLOT(saveSelection()));

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

    if (scene != NULL)
    {
        delete scene;
        scene = NULL;
    }
}

void MainWindow::calculate_bounding_rect(int vec1, int vec2)
{
    boundingRect.setTopRight(QPointF(x[0][vec1], x[0][vec2]));
    boundingRect.setBottomLeft(QPointF(x[0][vec1], x[0][vec2]));

    for (int i = 0; i < selectionSize; ++i)
    {
        if (x[i][vec1] < boundingRect.left())
            boundingRect.setLeft(x[i][vec1]);

        if (x[i][vec1] > boundingRect.right())
            boundingRect.setRight(x[i][vec1]);

        if (x[i][vec2] < boundingRect.bottom())
            boundingRect.setBottom(x[i][vec2]);

        if (x[i][vec2] >  boundingRect.top())
            boundingRect.setTop(x[i][vec2]);
    }

}

void MainWindow::calculate_values()
{
    if (!inputDataLoaded)
        return;

    int vec1 = ui.component1->value() - 1;
    int vec2 = ui.component2->value() - 1;

    middleX = 0.0;
    middleY = 0.0;
    for (int i = 0; i < selectionSize; ++i)
    {
        middleX += x[i][vec1];
        middleY += x[i][vec2];
    }
    middleX = middleX/static_cast<double>(selectionSize);
    middleY = middleY/static_cast<double>(selectionSize);

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

    r = kxy/(sigmaX*sigmaY);

    ui.labelSigmaX->setText(QString("%1").arg(sigmaX));
    ui.labelSigmaY->setText(QString("%1").arg(sigmaY));
    ui.labelKxy->setText(QString("%1").arg(kxy));
    ui.labelR->setText(QString("%1").arg(r));

    draw();
}

double MainWindow::calculate_y1(double x, double p)
{
    double sx2 = sigmaX*sigmaX;
    double sy2 = sigmaY*sigmaY;
    
    double lambdad = -log(p);//fabs(2.0*(1.0 - r*r)*log(2.0*M_PI*sigmaX*sigmaY*sqrt(1.0 - r*r)*p));

//    qDebug() << "f=" << (r*sigmaY*x + sigmaX*middleY - r*sigmaY*middleX)
//            << " s1=" << r*r*sy2*x*x
//            << " s2=" << 2.0*r*r*sy2*x*middleX
//            << " s3=" << r*r*sy2*middleX*middleX
//            << " s4=" << sy2*x*x
//            << " s5=" << 2.0*sy2*x*middleX
//            << " s6=" << sy2*middleX*middleX
//            << " s7=" << lambdad*sx2*sy2
//            << " sresult=" << (r*r*sy2*x*x - 2.0*r*r*sy2*x*middleX +
//                 r*r*sy2*middleX*middleX - sy2*x*x +
//                 2.0*sy2*x*middleX - sy2*middleX*middleX +
//                 lambdad*sx2*sy2);


    return (r*sigmaY*x + sigmaX*middleY - r*sigmaY*middleX +
            sqrt(r*r*sy2*x*x - 2.0*r*r*sy2*x*middleX +
                 r*r*sy2*middleX*middleX - sy2*x*x +
                 2.0*sy2*x*middleX - sy2*middleX*middleX +
                 lambdad*sx2*sy2)
            )/sigmaX;
}

double MainWindow::calculate_y2(double x, double p)
{
    double sx2 = sigmaX*sigmaX;
    double sy2 = sigmaY*sigmaY;

    double lambdad = -log(p);//fabs(2.0*(1.0 - r*r)*log(2.0*M_PI*sigmaX*sigmaY*sqrt(1.0 - r*r)*p));

    return (r*sigmaY*x + sigmaX*middleY - r*sigmaY*middleX -
            sqrt(r*r*sy2*x*x - 2.0*r*r*sy2*x*middleX +
                 r*r*sy2*middleX*middleX - sy2*x*x +
                 2.0*sy2*x*middleX - sy2*middleX*middleX +
                 lambdad*sx2*sy2)
            )/sigmaX;
}

double MainWindow::drand()
{
    return static_cast<double>(rand())/static_cast<double>(RAND_MAX);
}

void MainWindow::draw()
{
    if (scene)
    {
        delete scene;
        scene = NULL;
    }

    int vec1 = ui.component1->value() - 1;
    int vec2 = ui.component2->value() - 1;

    calculate_bounding_rect(vec1, vec2);

    shiftX = 0.0;
    shiftY = 0.0;
    QPen pen;

    scene = new QGraphicsScene(boundingRect, this);

    //draw axies lines
    pen.setColor(QColor(0, 0, 0, 255));
    pen.setWidth(1);
    scene->addLine(plot_x(0.0), plot_y(0.0), plot_x(1.0), plot_y(0.0), pen);
    scene->addLine(plot_x(0.0), plot_y(0.0), plot_x(0.0), plot_y(1.0), pen);

    //draw all points
    if (ui.checkBoxSelection->isChecked())
    {
        pen.setColor(QColor(255, 0, 0, 255));
        pen.setWidth(2);
        for (int i = 0; i < selectionSize; ++i)
            scene->addLine(plot_x(x[i][vec1]), plot_y(x[i][vec2]), plot_x(x[i][vec1]), plot_y(x[i][vec2]), pen);
    }

    //draw middle point
    if (ui.checkBoxMiddle->isChecked())
    {
        pen.setColor(QColor(0, 0, 255, 255));
        pen.setWidth(3);
        scene->addLine(plot_x(middleX), plot_y(middleY), plot_x(middleX), plot_y(middleY), pen);
    }

    //draw isolines
    if (ui.checkBoxIsolines->isChecked())
    {
        pen.setColor(QColor(0, 0, 255, 255));
        pen.setWidth(1);
        QPainterPath path1;
        QPainterPath path2;
        QPainterPath path3;
        draw_ellipse(path1, 0.3);
        draw_ellipse(path2, 0.6);
        draw_ellipse(path3, 0.9);
        scene->addPath(path1, pen);
        scene->addPath(path2, pen);
        scene->addPath(path3, pen);
    }

    scene->setBackgroundBrush(QBrush(QColor(0, 255, 0, 255)));
    ui.graphicsView->setScene(scene);
    ui.graphicsView->show();
}

void MainWindow::draw_ellipse(QPainterPath &path, double p)
{

    double width = boundingRect.right() - boundingRect.left();
    width /= 100.0;
    double left = boundingRect.left();

    bool first = true;
    for (int i = 0; i <= 100; ++i)
    {
        double currentX = left + static_cast<double>(i)*width;
        double currentY = calculate_y1(currentX, p);
        if (first)
        {
            if (!(isnan(currentY) || isinf(currentY)))
            {
                path.moveTo(plot_x(currentX), plot_y(currentY));
                first = false;
            }
        }
        else
            if (!(isnan(currentY) || isinf(currentY)))
                path.lineTo(plot_x(currentX), plot_y(currentY));
    }

    for (int i = 100; i >= 0; --i)
    {
        double currentX = left + static_cast<double>(i)*width;
        double currentY = calculate_y2(currentX, p);
        if (!(isnan(currentY) || isinf(currentY)))
            path.lineTo(plot_x(currentX), plot_y(currentY));
    }
    path.connectPath(path);
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

    selectionGenerated = true;
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
}

void MainWindow::load()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select the input file"));

    if (filename.isEmpty())
        return;

    //open file
    std::fstream in;
    in.open(filename.toAscii().data(), std::ios_base::in);
    if (!in.is_open())
        return;

    //read dimention of X
    in >> m;

    ui.component1->setMaximum(m);
    ui.component2->setMaximum(m);

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
    selectionGenerated = false;
}

double MainWindow::plot_x(double x)
{
    return x*static_cast<double>(ui.zoomX->value())*10.0;
}

double MainWindow::plot_y(double y)
{
    return -y*static_cast<double>(ui.zoomY->value())*10.0;
}

void MainWindow::saveSelection()
{
    if (!inputDataLoaded)
    {
        QMessageBox::critical(this, tr("Input data not loaded"),
                              tr("Input data not loaded!"),
                              QMessageBox::Ok);
        return;
    }

    if (!selectionGenerated)
    {
        QMessageBox::critical(this, tr("Selection doesn't generated"),
                              tr("Selection doesn't generated!"),
                              QMessageBox::Ok);
        return;
    }

    if (x == NULL)
        return;

    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Choose file to save selection");

    if (filename.isEmpty())
        return;

    //open file
    std::fstream out;
    out.open(filename.toAscii().data(), std::ios_base::out);
    if (!out.is_open())
        return;

    out << m << std::endl;
    out << selectionSize << std::endl;

    for (int i = 0; i < selectionSize; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            out << x[i][j] << " ";
        }
        out << std::endl;
    }

    out.close();
}
