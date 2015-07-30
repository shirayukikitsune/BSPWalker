#include "camera.h"

Camera::Camera()
{
    eye = QVector3D(0.0, 0.0, 0.0);
    forward = QVector3D(0.0, 0.0, -1.0);
    up = QVector3D(0.0, 1.0, 0.0);
    right = QVector3D(1.0, 0.0, 0.0);

    tracking = false;
}

QMatrix4x4 Camera::getView()
{
    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, eye + forward, up);
    return view;
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
    float yaw = (mouseClickPos.x() - pos.x()) / 2.0f;
    float pitch = (mouseClickPos.y() - pos.y()) / 2.0f;

    forward = QQuaternion::fromAxisAndAngle(right, pitch).rotatedVector(forward);
    forward = QQuaternion::fromAxisAndAngle(up, yaw).rotatedVector(forward);
    up = QVector3D::crossProduct(right, forward);
    right = QVector3D::crossProduct(forward, up);
}

void Camera::walk(float speed)
{
    eye += forward * speed;
}

void Camera::strafe(float speed)
{
    eye += right * speed;
}
