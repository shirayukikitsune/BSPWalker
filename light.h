#ifndef LIGHT_H
#define LIGHT_H

#include <QVector3D>

class Light
{
public:
    Light();
    ~Light();

    QVector3D direction;
    QVector3D color;
    float intensity;
};

#endif // LIGHT_H
