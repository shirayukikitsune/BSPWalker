#include "openglwidget.h"

#include <QFileDialog>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    bsp = nullptr;
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    update();
}

void OpenGLWidget::loadBSP()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select BSP File", QString(), QString("BSP Files (*.bsp)"));

    if (fileName.isEmpty())
        return;

    if (bsp)
        delete bsp;

    bsp = new BSP();
    connect(bsp, SIGNAL(loadError(QString)), this, SLOT(bspError(QString)));
    bsp->loadMap(fileName);
}

void OpenGLWidget::bspError(QString error)
{
    emit setStatusBarMessage(error);
}
