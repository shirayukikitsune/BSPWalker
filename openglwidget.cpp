#include "openglwidget.h"

#include <iostream>
#include <sstream>

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
    projection.perspective(45.0f, (float)w / (float)h, 0.1f, 10000.0f);
    //projection.ortho(0, 1000, 0, 100, 0.01f, 5000.0f);

    QPointF screenCenter(w / 2, h / 2);

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

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!this->hasFocus())
        return;

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

void OpenGLWidget::focusInEvent(QFocusEvent *)
{
    setCursor(QCursor(Qt::BlankCursor));

    camera.beginMouseTrack(QPoint(size().width() / 2, size().height() / 2));

    emit setStatusBarMessage("Press ESC to regain mouse control");
}

void OpenGLWidget::focusOutEvent(QFocusEvent *)
{
    setCursor(QCursor(Qt::ArrowCursor));

    camera.endMouseTrack();

    emit setStatusBarMessage("");
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

    // Try to find the entity that holds the position and angles for the initial camera position
    BSPEntity *entity = bsp->findEntityByClassname("info_player_intermission");
    if (!entity)
        camera.setPosition(bsp->getCenter());
    else {
        std::istringstream iss(entity->getSetting("origin").toLatin1().data());
        float x, y, z;

        iss >> x >> y >> z;
        camera.setPosition(QVector3D(x, y, z));

        iss.seekg(0);
        iss.str(entity->getSetting("angles").toLatin1().data());
        iss >> x >> y >> z;

        camera.setRotation(x, y, z);
    }
}

void OpenGLWidget::bspError(QString error)
{
    emit setStatusBarMessage(error);
}
