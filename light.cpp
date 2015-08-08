#include "light.h"

Light::Light()
{
    direction = QVector3D(0.0, 0.0, 1.0);
    color = QVector3D(1.0, 1.0, 1.0);
    intensity = 64;
}

Light::~Light()
{

}

