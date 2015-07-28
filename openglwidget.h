#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "bsp.h"

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

private:
    BSP *bsp;
    QTimer timer;

public slots:
    void loadBSP();
    void bspError(QString error);

signals:
    void setStatusBarMessage(QString);
};

#endif // OPENGLWIDGET_H
