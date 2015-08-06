#ifndef BSPSHADER_H
#define BSPSHADER_H

#include <QOpenGLTexture>
#include <QString>

class BSPShader
{
public:
    BSPShader();
    ~BSPShader();

    /**
     * @brief Allocates all GPU resources, creating from a texture file.
     *
     * @remarks This will create only the albedo channel, with default parameters
     */
    bool createFromTextureFile(const QString& textureFile);

    /**
     * @brief Allocates all GPU resources, creating from a .shader section
     *
     * @remarks This is the preferred method, since sets specific parameters for each texture
     */
    bool createFromShaderText(const QString& shaderText);

    /**
     * @brief Binds the GPU resources for use
     *
     * @remarks Albedo texture goes to texture unit 0
     */
    void bind();

    /**
     * @brief Destroys all GPU resources
     */
    void destroy();

    /**
     * @brief Unbinds all GPU resources
     */
    void release();

private:
    QOpenGLTexture *albedo;
};

#endif // BSPSHADER_H
