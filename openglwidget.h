#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "bsp.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class OpenGLWidget : public QOpenGLWidget, private QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = 0);

protected:
    void initializeGL();
    void resizeGL(int w, int h);

private:
    BSP *bsp;

public slots:
    void loadBSP();
    void bspError(QString error);

signals:
    void setStatusBarMessage(QString);
};

#endif // OPENGLWIDGET_H
