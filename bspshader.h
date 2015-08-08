#ifndef BSPSHADER_H
#define BSPSHADER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QString>

class BSPShader
{
public:
    BSPShader(const QString& name);
    ~BSPShader();

    const QString& getName() const { return name; }

    /**
     * @brief Allocates all GPU resources, creating from a texture file.
     *
     * @remarks This will create only the albedo channel, with default parameters
     */
    bool create();

    /**
     * @brief Binds the GPU resources for use
     *
     * @remarks Albedo texture goes to texture unit 0
     */
    void bind(QOpenGLShaderProgram *shaderProgram);

    /**
     * @brief Destroys all GPU resources
     */
    void destroy();

    /**
     * @brief Unbinds all GPU resources
     */
    void release();

    void update();

    void setAlbedo(const QString &file);
    void setUVModValue(QVector2D uvModValue);

private:
    QOpenGLTexture *albedo;
    QString name;

    QVector2D uvMod;
    QVector2D uvModValue;
};

#endif // BSPSHADER_H
