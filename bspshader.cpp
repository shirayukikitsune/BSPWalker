#include "bspshader.h"

#include <QFile>

BSPShader::BSPShader()
{
    albedo = nullptr;
}

BSPShader::~BSPShader()
{
    release();
    destroy();
}

bool BSPShader::createFromTextureFile(const QString& textureFile)
{
    // Check if a texture exists, appending .tga and .jpg to the path
    QString tgaTex = QString("%1.%2").arg(textureFile, "tga"), jpgTex = QString("%1.%2").arg(textureFile, "jpg");

    QImage texImage;

    // Check if the TARGA texture exists and is loadable
    if (QFile::exists(tgaTex)) {
        if (!texImage.load(tgaTex))
            return false;
    }
    // Now check for JPEG
    else if (QFile::exists(jpgTex)) {
        if (!texImage.load(jpgTex))
            return false;
    }
    // If neither exists, we must try to parse the shaderText from a shader file
    else return false;

    albedo = new QOpenGLTexture(texImage);
    albedo->create();

    return true;
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
