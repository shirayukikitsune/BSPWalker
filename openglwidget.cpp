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
    glViewport(0, 0, w, h);

    projection.setToIdentity();
    projection.perspective(60.0f, (float)w / (float)h, 0.1f, 10000.0f);

    QPointF screenCenter(w / 2, h / 2);
    camera.beginMouseTrack(screenCenter);

    update();
}

void OpenGLWidget::paintGL()
{
    if (!bsp)
        return;

    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bsp->render(camera.getView(), projection);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    //if (event->button() & Qt::LeftButton)
     //   camera.beginMouseTrack(event->localPos());
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //if (event->button() & Qt::LeftButton)
      ///  camera.endMouseTrack();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    camera.mouseMove(event->localPos());

    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        camera.walk( 32.0f); break;
    case Qt::Key_Down:
    case Qt::Key_S:
        camera.walk(-32.0f); break;
    case Qt::Key_Left:
    case Qt::Key_A:
        camera.strafe(-32.0f); break;
    case Qt::Key_Right:
    case Qt::Key_D:
        camera.strafe( 32.0f); break;
    case Qt::Key_Escape:
        this->clearFocus(); break;
    }
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

    camera.setPosition(bsp->getCenter());
}

void OpenGLWidget::bspError(QString error)
{
    emit setStatusBarMessage(error);
}
