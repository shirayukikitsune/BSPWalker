#include "bspshader.h"

BSPShader::BSPShader()
{
    albedo = nullptr;
}

BSPShader::~BSPShader()
{
    release();
    destroy();
}

void BSPShader::create(const QString& shaderText)
{

}

void BSPShader::bind()
{
    if (albedo)
        albedo->bind(0);
}

void BSPShader::destroy()
{
    if (albedo) {
        albedo->destroy();
        delete albedo;
        albedo = nullptr;
    }
}

void BSPShader::release()
{
    if (albedo) {
        albedo->release(0);
    }
}
