#include "postprocesseffect.h"

PostProcessEffect::PostProcessEffect()
{
    vertexShader = nullptr;
    fragmentShader = nullptr;
    shaderProgram = nullptr;

    vao = nullptr;
    vboVertices = nullptr;
    vboTextureCoords = nullptr;
    vboIndices = nullptr;

    initializeOpenGLFunctions();
}

PostProcessEffect::~PostProcessEffect()
{
    destroyEffect();
}

void PostProcessEffect::setScreenDimentions(const QVector2D &dimentions)
{
    screenDimentions = dimentions;
}

void PostProcessEffect::render(GLuint original, GLuint input, GLuint depth)
{
    // Use the linked shader program
    shaderProgram->bind();

    setEffectUniforms();

    shaderProgram->setUniformValue("originalTexture", 0);
    shaderProgram->setUniformValue("chainedTexture", 1);
    shaderProgram->setUniformValue("depthTexture", 2);
    shaderProgram->setUniformValue("screenDimentions", screenDimentions);
    shaderProgram->setUniformValue("texelSize", QVector2D(1.0 / screenDimentions.x(), 1.0 / screenDimentions.y()));

    vao->bind();
    vboIndices->bind();

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depth);

    // Bind the output of the last post-process effect of the chain to the texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, input);

    // Bind the unmodified texture to the texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, original);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Release used resources
    vboIndices->release();
    vao->release();
    shaderProgram->release();
}

void PostProcessEffect::createEffect(const QString &fragmentShaderFile, const QString &vertexShaderFile)
{
    destroyEffect();

    vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    if (!vertexShader->compileSourceFile(vertexShaderFile))
        qWarning() << vertexShader->log() << endl;

    fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    if (!fragmentShader->compileSourceFile(fragmentShaderFile))
        qWarning() << fragmentShader->log() << endl;

    shaderProgram = new QOpenGLShaderProgram;
    shaderProgram->addShader(vertexShader);
    shaderProgram->addShader(fragmentShader);

    if (!shaderProgram->link())
        qWarning() << shaderProgram->log() << endl;

    vertices = new QVector4D[4];
    vertices[0] = QVector4D(-1.0f, -1.0f, 0, 1);
    vertices[1] = QVector4D( 1.0f, -1.0f, 0, 1);
    vertices[2] = QVector4D(-1.0f,  1.0f, 0, 1);
    vertices[3] = QVector4D( 1.0f,  1.0f, 0, 1);

    texCoords = new QVector2D[4];
    texCoords[0] = QVector2D(0, 0);
    texCoords[1] = QVector2D(1, 0);
    texCoords[2] = QVector2D(0, 1);
    texCoords[3] = QVector2D(1, 1);

    indices = new unsigned int[6];
    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 1; indices[4] = 2; indices[5] = 3;

    vao = new QOpenGLVertexArrayObject;
    vao->create();
    vao->bind();

    vboVertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboVertices->create();
    vboVertices->bind();
    vboVertices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboVertices->allocate(vertices, 4 * sizeof(QVector4D));
    delete[] vertices;
    vertices = nullptr;

    vboTextureCoords = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboTextureCoords->create();
    vboTextureCoords->bind();
    vboTextureCoords->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboTextureCoords->allocate(texCoords, 4 * sizeof(QVector2D));
    delete[] texCoords;
    texCoords = nullptr;

    shaderProgram->bind();

    vboVertices->bind();
    shaderProgram->enableAttributeArray("vPosition");
    shaderProgram->setAttributeBuffer("vPosition", GL_FLOAT, 0, 4, 0);
    vboTextureCoords->bind();
    shaderProgram->enableAttributeArray("vTexCoord");
    shaderProgram->setAttributeBuffer("vTexCoord", GL_FLOAT, 0, 2, 0);

    vao->release();

    vboIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vboIndices->create();
    vboIndices->bind();
    vboIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboIndices->allocate(indices, 6 * sizeof(unsigned int));
    delete[] indices;
    indices = nullptr;
}

void PostProcessEffect::destroyEffect()
{
    delete vertexShader;
    vertexShader = nullptr;

    delete fragmentShader;
    fragmentShader = nullptr;

    if (shaderProgram) {
        shaderProgram->release();
        delete shaderProgram;
        shaderProgram = nullptr;
    }

    if (vao) {
        vao->release();
        delete vao;
        vao = nullptr;
    }

    if (vboVertices) {
        vboVertices->release();
        delete vboVertices;
        vboVertices = nullptr;
    }

    if (vboTextureCoords) {
        vboTextureCoords->release();
        delete vboTextureCoords;
        vboTextureCoords = nullptr;
    }

    if (vboIndices) {
        vboIndices->release();
        delete vboIndices;
        vboIndices = nullptr;
    }
}

void PassthroughEffect::create()
{
    createEffect(":/shaders/effects/fshader.glsl", ":/shaders/effects/vshader.glsl");
}

QString PassthroughEffect::toString() const
{
    return QString("Passthrough Effect");
}

EdgeDetectionEffect::EdgeDetectionEffect()
{
    addOriginal = false;
    intensity = 1.0f;
}

void EdgeDetectionEffect::create()
{
    createEffect(":/shaders/effects/edge_detection.glsl");
}

void EdgeDetectionEffect::setIntensity(float value)
{
    intensity = value;
}

void EdgeDetectionEffect::setAdditive(bool value)
{
    addOriginal = value;
}

void EdgeDetectionEffect::setEffectUniforms()
{
    shaderProgram->setUniformValue("addOriginal", addOriginal);
    shaderProgram->setUniformValue("intensity", intensity);
}

QString EdgeDetectionEffect::toString() const
{
    return QString("Frei-Chen Edge Detection%1; Intensity %2").arg(addOriginal ? " with Overlay" : "").arg(intensity, 1, 'f', 1, '0');
}

CrossStitchEffect::CrossStitchEffect()
{
    inverted = 0;
    stitchSize = 6.0f;
}

void CrossStitchEffect::create()
{
    createEffect(":/shaders/effects/cross_stitch.glsl");
}

void CrossStitchEffect::setStitchSize(float value)
{
    stitchSize = value;
}

void CrossStitchEffect::setInverted(bool value)
{
    inverted = value ? 1 : 0;
}

void CrossStitchEffect::setEffectUniforms()
{
    shaderProgram->setUniformValue("stitchSize", stitchSize);
    shaderProgram->setUniformValue("inverted", inverted);
}

QString CrossStitchEffect::toString() const
{
    return QString("%1 Cross Stitch; Size %2").arg(inverted ? "Inverted" : "Normal").arg(stitchSize, 1, 'f', 1, '0');
}

void GaussianBlurEffect::setEffectUniforms()
{
    shaderProgram->setUniformValueArray("gaussianWeights", values, 15, 1);
}

void HorizontalGaussianBlurEffect::create()
{
    createEffect(":/shaders/effects/gaussian_blur_frag.glsl", ":/shaders/effects/horizontal_gaussian_blur_vertex.glsl");
}

QString HorizontalGaussianBlurEffect::toString() const
{
    return QString("Horizontal Gaussian Blur; Intensity %1").arg(getSigma(), 1, 'f', 2, '0');
}

void VerticalGaussianBlurEffect::create()
{
    createEffect(":/shaders/effects/gaussian_blur_frag.glsl", ":/shaders/effects/vertical_gaussian_blur_vertex.glsl");
}

QString VerticalGaussianBlurEffect::toString() const
{
    return QString("Vertical Gaussian Blur; Intensity %1").arg(getSigma(), 1, 'f', 2, '0');
}

GammaCorrectionEffect::GammaCorrectionEffect()
{
    gammaFactor = QVector3D(1.0f, 1.0f, 1.0f);
}

void GammaCorrectionEffect::create()
{
    createEffect(":/shaders/effects/gamma_correction.glsl");
}

void GammaCorrectionEffect::setGammaFactor(QVector3D value)
{
    gammaFactor = value;
}

void GammaCorrectionEffect::setEffectUniforms()
{
    shaderProgram->setUniformValue("gammaFactor", gammaFactor);
}

QString GammaCorrectionEffect::toString() const
{
    return QString("Gamma Correction; (R: %1; G: %2; B: %3)").arg(gammaFactor.x(), 1, 'f', 2, '0').arg(gammaFactor.y(), 1, 'f', 2, '0').arg(gammaFactor.z(), 1, 'f', 2, '0');
}

BloomEffect::BloomEffect()
{
    bloomIntensity = 0.0f;
    bloomSaturation = 1.0f;
    originalIntensity = 1.0f;
    originalSaturation = 1.0f;
}

void BloomEffect::setBloomIntensity(float intensity)
{
    bloomIntensity = intensity;
}

void BloomEffect::setBloomSaturation(float saturation)
{
    bloomSaturation = saturation;
}

void BloomEffect::setOriginalIntensity(float intensity)
{
    originalIntensity = intensity;
}

void BloomEffect::setOriginalSaturation(float saturation)
{
    originalSaturation = saturation;
}

void BloomEffect::setEffectUniforms()
{
    shaderProgram->setUniformValue("bloomIntensity", bloomIntensity);
    shaderProgram->setUniformValue("bloomSaturation", bloomSaturation);
    shaderProgram->setUniformValue("originalIntensity", originalIntensity);
    shaderProgram->setUniformValue("originalSaturation", originalSaturation);
}

void BloomEffect::create()
{
    createEffect(":/shaders/effects/bloom.glsl");
}

QString BloomEffect::toString() const
{
    return QString("Bloom; (%1; %2; %3; %4)")
            .arg(bloomIntensity, 1, 'f', 2, '0')
            .arg(bloomSaturation, 1, 'f', 2, '0')
            .arg(originalIntensity, 1, 'f', 2, '0')
            .arg(originalSaturation, 1, 'f', 2, '0');
}

DepthOfFieldEffect::DepthOfFieldEffect()
{
    fStop = 22.0f;
    focalLength = 10.0f;
}

void DepthOfFieldEffect::setEffectUniforms()
{
    shaderProgram->setUniformValue("focalLength", focalLength);
    shaderProgram->setUniformValue("fstop", fStop);
}

void DepthOfFieldEffect::create()
{
    createEffect(":/shaders/effects/dof.glsl");
}

QString DepthOfFieldEffect::toString() const
{
    return QString("DoF; (%1; %2)")
            .arg(fStop, 1, 'f', 2, '0')
            .arg(focalLength, 1, 'f', 2, '0');
}
