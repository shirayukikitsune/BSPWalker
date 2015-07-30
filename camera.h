#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix4x4>
#include <QVector3D>

class Camera
{
public:
    Camera();

    QMatrix4x4 getView();

    void setPosition(QVector3D pos) { eye = pos; }

    void beginMouseTrack(const QPointF &pos);
    void endMouseTrack();
    void mouseMove(const QPointF &pos);

    /**
     * @brief Moves the camera forwards or backwards, applying a rotation
     */
    void walk(float speed);
    void strafe(float speed);

private:
    QPointF mouseClickPos;
    bool tracking;

    QVector3D eye;
    QVector3D forward;
    QVector3D up;
    QVector3D right;
};

#endif // CAMERA_H
