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
     * @brief Allocates all GPU resources
     */
    bool create(const QString& shaderText);
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
