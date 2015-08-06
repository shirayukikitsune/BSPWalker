#ifndef POSTPROCESSEFFECT_H
#define POSTPROCESSEFFECT_H

#include <QtMath>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

/// @brief Base class for all post-processing effects
/// @remarks This is an abstract class, meaning that all effects must override at least the `create` and `setEffectUniforms` methods
class PostProcessEffect : protected QOpenGLFunctions
{
public:
    PostProcessEffect();
    virtual ~PostProcessEffect();

    /// @brief Renders the effect to the output Framebuffer Object
    ///
    /// This function will call the respective fragment and vertex shaders for the current effect
    void render(GLuint original, GLuint input, GLuint depth);

    /// @brief Initializes the current PostProcess effect
    virtual void create() = 0;

    /// @brief Sets the screen dimentions to the shaders
    void setScreenDimentions(const QVector2D &dimentions);

    /// @brief Converts this effect to a description string
    virtual QString toString() const = 0;

protected:
    /// @brief Creates the shaders
    void createEffect(const QString &fragmentShaderFile, const QString &vertexShaderFile = ":/shaders/vshader.glsl");
    /// @brief Destroys the shaders
    void destroyEffect();

    /// @brief Defines the effect uniforms
    virtual void setEffectUniforms() {}

    QOpenGLShaderProgram *shaderProgram;

private:
    QOpenGLShader *vertexShader;
    QOpenGLShader *fragmentShader;

    QOpenGLVertexArrayObject *vao;
    QOpenGLBuffer *vboVertices;
    QOpenGLBuffer *vboIndices;
    QOpenGLBuffer *vboTextureCoords;

    QVector4D *vertices;
    QVector2D *texCoords;
    unsigned int *indices;

    QVector2D screenDimentions;
};

class PassthroughEffect : public PostProcessEffect
{
public:
    virtual void create();

    virtual QString toString() const;
};

class EdgeDetectionEffect : public PostProcessEffect
{
public:
    EdgeDetectionEffect();

    void setIntensity(float value);
    float getIntensity() const { return intensity; }

    void setAdditive(bool value);
    bool getAdditive() const { return addOriginal; }

    virtual void create();

    virtual QString toString() const;

protected:
    virtual void setEffectUniforms();

private:
    bool addOriginal;
    float intensity;
};

class CrossStitchEffect : public PostProcessEffect
{
public:
    CrossStitchEffect();

    void setInverted(bool value);
    bool getInverted() const { return inverted; }

    void setStitchSize(float value);
    float getStitchSize() const { return stitchSize; }

    virtual void create();

    virtual QString toString() const;

protected:
    virtual void setEffectUniforms();

private:
    int inverted;
    float stitchSize;
};

/// @brief This class defines a gaussian used for probability density function
/// @remarks Since this is a PDF, the MI parameter is always set to zero.
/// @remarks The sample count is hardcoded to 15.
class GaussianBlurEffect : public PostProcessEffect
{
public:
    GaussianBlurEffect() {
        sigma = 1.0f;
    }

    void setSigma(float value) {
        sigma = value;
        calculateValues();
    }

    float getSigma() const { return sigma; }

protected:
    float values[15];

    virtual void setEffectUniforms();

private:
    void calculateValues() {
        auto g = [this](float x) -> float {
            // Square root of 2 PI
            static float sqrt_2pi = 2.50662827463f;
            return 1.0 / (sigma * sqrt_2pi) * expf(-0.5 * x * x / (sigma * sigma));
        };

        float weightSum = 0.0f;
        int i;
        // Simple optimization - since gaussian functions are symmetric, calculate only for half of the function
        for (i = 0; i < 15; ++i) {
            values[i] = g(i - 7);
            weightSum += values[i];
        }

        // Normalize the weights
        for (i = 0; i < 15; ++i)
            values[i] /= weightSum;
    }

    float sigma;
};

class HorizontalGaussianBlurEffect : public GaussianBlurEffect
{
public:
    virtual void create();

    virtual QString toString() const;
};

class VerticalGaussianBlurEffect : public GaussianBlurEffect
{
public:
    virtual void create();

    virtual QString toString() const;
};

class GammaCorrectionEffect : public PostProcessEffect
{
public:
    GammaCorrectionEffect();

    void setGammaFactor(QVector3D value);
    const QVector3D& getGammaFactor() const { return gammaFactor; }

    virtual void create();

    virtual QString toString() const;

protected:
    virtual void setEffectUniforms();

private:
    QVector3D gammaFactor;
};

class BloomEffect : public PostProcessEffect
{
public:
    BloomEffect();

    void setBloomSaturation(float saturation);
    float getBloomSaturation() const { return bloomSaturation; }

    void setBloomIntensity(float intensity);
    float getBloomIntensity() const { return bloomIntensity; }

    void setOriginalSaturation(float saturation);
    float getOriginalSaturation() const { return originalSaturation; }

    void setOriginalIntensity(float intensity);
    float getOriginalIntensity() const { return originalIntensity; }

    virtual void create();

    virtual QString toString() const;

protected:
    virtual void setEffectUniforms();

private:
    float bloomSaturation;
    float bloomIntensity;
    float originalSaturation;
    float originalIntensity;
};

#endif // POSTPROCESSEFFECT_H
