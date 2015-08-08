#include "bspshader.h"

#include <QFile>

BSPShader::BSPShader(const QString &name)
    : name(name)
{
    albedo = nullptr;
}

BSPShader::~BSPShader()
{
    release();
    destroy();
}

bool BSPShader::create()
{
    // Check if a texture exists, appending .tga and .jpg to the path
    QString tgaTex = QString("%1.%2").arg(name, "tga"), jpgTex = QString("%1.%2").arg(name, "jpg");

    // Check if the TARGA texture exists and is loadable
    if (QFile::exists(tgaTex)) {
        setAlbedo(tgaTex);
        return true;
    }
    // Now check for JPEG
    else if (QFile::exists(jpgTex)) {
        setAlbedo(jpgTex);
        return true;
    }
    // If neither exists, we must try to parse the shaderText from a shader file
    return false;
}

void BSPShader::setAlbedo(const QString &file)
{
    QImage texImage(file);

    albedo = new QOpenGLTexture(texImage);
    albedo->create();
}

void BSPShader::bind(QOpenGLShaderProgram *shaderProgram)
{
    if (albedo)
        albedo->bind(0);

    shaderProgram->setUniformValue("uvMod", uvMod);
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

void BSPShader::update()
{
    uvMod += uvModValue;
}

void BSPShader::setUVModValue(QVector2D uvModValue)
{
    this->uvModValue = uvModValue;
}
