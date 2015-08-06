#include "camera.h"

#include <QtMath>

Camera::Camera()
{
    position = QVector3D(0.0, 0.0, 0.0);
    pitch = yaw = roll = 0.0f;

    update();

    tracking = false;
}

QMatrix4x4 Camera::getView()
{
    QMatrix4x4 view;
    view.lookAt(position, position + forward, up);
    return view;
}

void Camera::setRotation(float pitch, float yaw, float roll)
{
    this->pitch = qDegreesToRadians(pitch);
    this->yaw = qDegreesToRadians(yaw);
    this->roll = qDegreesToRadians(roll);

    update();
}

void Camera::beginMouseTrack(const QPointF &pos)
{
    mouseClickPos = pos;
    tracking = true;
}

void Camera::endMouseTrack()
{
    tracking = false;
}

void Camera::mouseMove(const QPointF &pos)
{
    float yaw = (pos.x() - mouseClickPos.x()) / 2.0f;
    float pitch = (pos.y() - mouseClickPos.y()) / 2.0f;

    this->pitch -= pitch;
    this->pitch = qBound<float>(-90, this->pitch, 90);
    this->yaw -= yaw;

    update();
}

void Camera::update()
{
    QMatrix4x4 r;
    r.rotate(yaw, 0, 0, 1);
    r.rotate(roll, 0, 1, 0);
    r.rotate(pitch, 1, 0, 0);
    // BSP uses Z as up
    up = r * QVector3D(0.0f, 0.0f, 1.0f);
    forward = r * QVector3D(0.0f, 1.0f, 0.0f);
}

void Camera::walk(float speed)
{
    position += forward * speed;
}

void Camera::strafe(float speed)
{
    position += QVector3D::crossProduct(forward, up) * speed;
}
