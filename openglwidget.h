#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "bsp.h"
#include "camera.h"

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>

class OpenGLWidget : public QOpenGLWidget, private QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = 0);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    BSP *bsp;
    QTimer timer;
    Camera camera;
    QMatrix4x4 modelView;
    QMatrix4x4 projection;

public slots:
    void loadBSP();
    void bspError(QString error);

signals:
    void setStatusBarMessage(QString);
};

#endif // OPENGLWIDGET_H
