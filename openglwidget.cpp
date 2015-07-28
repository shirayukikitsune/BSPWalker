#include "openglwidget.h"

#include <iostream>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMouseEvent>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    bsp = nullptr;
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    std::cout << "GL Version " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start(0);

    glEnable(GL_DEPTH_TEST);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    projection.setToIdentity();
    projection.perspective(60.0f, (float)w / (float)h, 0.1f, 1000.0f);

    update();
}

void OpenGLWidget::paintGL()
{
    if (!bsp)
        return;

    makeCurrent();

    modelView.setToIdentity();
    modelView.lookAt(camera.eye, camera.at, camera.up);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bsp->render(modelView, projection);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{

}

void OpenGLWidget::loadBSP()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select BSP File", QString(), QString("BSP Files (*.bsp)"));

    if (fileName.isEmpty())
        return;

    if (bsp)
        delete bsp;

    makeCurrent();
    bsp = new BSP();
    connect(bsp, SIGNAL(loadError(QString)), this, SLOT(bspError(QString)));
    bsp->loadMap(fileName);
}

void OpenGLWidget::bspError(QString error)
{
    emit setStatusBarMessage(error);
}
