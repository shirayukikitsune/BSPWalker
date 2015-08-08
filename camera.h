#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix4x4>
#include <QVector3D>

class Camera
{
public:
    Camera();

    QMatrix4x4 getView();

    QVector3D getPosition() { return position; }
    void setPosition(QVector3D pos) { position = pos; }
    void setRotation(float pitch, float yaw, float roll);

    void beginMouseTrack(const QPointF &pos);
    void endMouseTrack();
    void mouseMove(const QPointF &pos);

    /**
     * @brief Moves the camera forwards or backwards, applying a rotation
     */
    void walk(float speed);
    void strafe(float speed);

private:
    void update();

    QPointF mouseClickPos;
    bool tracking;

    QVector3D position;
    QVector3D forward;
    QVector3D up;
    float pitch, yaw, roll;
};

#endif // CAMERA_H
