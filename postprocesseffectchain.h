#ifndef POSTPROCESSEFFECTCHAIN_H
#define POSTPROCESSEFFECTCHAIN_H

#include "postprocesseffect.h"

#include <QOpenGLFunctions_4_0_Core>

#include <deque>

/// @brief Defines a chain of post-processing effects
class PostProcessEffectChain : private QOpenGLFunctions_4_0_Core
{
public:
    PostProcessEffectChain();
    ~PostProcessEffectChain();

    /// @brief Initializes the effect chain's Framebuffer Objects
    ///
    /// This will allocate the Framebuffer Objects with the specified square dimention
    void create(int size);

    /// @brief Destroys the effect chain's Framebuffer Objects
    void destroy();

    /// @brief Resizes the FBOs
    ///
    /// This will get the largest dimention and round up to a potency of two
    void resize(QVector2D screenDimentions);

    /// @brief Adds an already created effect to the end of the chain
    void addEffect(PostProcessEffect *effect);

    /// @brief Inserts an already created effect to a zero-based index of the chain
    void insert(unsigned int index, PostProcessEffect *effect);

    /// @brief Removes all currently assigned effects
    /// @remarks This will free the associated resources (i.e., objects will be deleted)
    void clear();

    /// @brief Removes an effect at an specified index
    /// @remarks This will free the associated resource (i.e., object will be deleted)
    void remove(int index);

    /// @brief Returns an effect at the speficied index
    PostProcessEffect *at(int index);

    /// @brief Prepares to render to the ''original'' FBO
    /// @remarks This will call ''glViewport'' to the FBO size, so it must be reset to the window dimentions later.
    void beginScene();

    /// @brief Finishes the rendering to the ''original'' FBO
    void endScene();

    /// @brief Renders the effect chain
    void render();

    /// @brief Returns the output of the chain
    GLuint getOutputTexture();

    /// @brief Initializes the OpenGL functions
    void initializeGL();

    /// @brief Returns a const reference to the active effects
    /// @remarks Useable only to view the effects
    const std::deque<PostProcessEffect*>& getActiveEffects() const { return activeEffects; }

    /// @brief Returns the size of the textures
    int getTextureSize() const { return fboSize; }

private:
    /// @brief The framebuffer object used for ''input'', ''output'', ''original'' and ''depth''
    GLuint fbo;

    /// @brief The previous framebuffer object, removed at beginScene() and restored at endScene()
    GLint oldFbo;

    /// @brief The textures that are stored in the FBO
    GLuint textures[4];

    /// @brief The size of the FBO textures
    int fboSize;

    /// @brief The fbo attachment index that will be used for ''input''
    unsigned int currentFbo;

    /// @brief The effects on this chain
    std::deque<PostProcessEffect*> activeEffects;

    /// @brief The passthrough effect, used to display the chain
    PassthroughEffect *final;

    QVector2D screenDimentions;
};

#endif // POSTPROCESSEFFECTCHAIN_H
