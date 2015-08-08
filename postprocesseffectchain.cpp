#include "postprocesseffectchain.h"

#include <cassert>

PostProcessEffectChain::PostProcessEffectChain()
{
    currentFbo = 0;

    fbo = 0;
    oldFbo = 0;
    fboSize = 0;
    textures[0] = textures[1] = textures[2] = textures[3] = 0;
    final = nullptr;
}

PostProcessEffectChain::~PostProcessEffectChain()
{
    delete final;

    destroy();
}

void PostProcessEffectChain::create(int size)
{
    if (fboSize == size) return;

    destroy();

    fboSize = size;

    // Create the FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

    // Create the FBO textures
    glGenTextures(4, textures);

    for (int i = 0; i < 3; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // If wrapping, repeat the texture in both S and T coordinates
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set linear interpolation on the texture sampling
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Create the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        // And attach it to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, textures[i], 0);
    }

    // Create a texture for depth buffer
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    // Create the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    // Attach it to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[3], 0);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

void PostProcessEffectChain::destroy()
{
    if (textures[0] != 0) {
        glDeleteTextures(4, textures);
        textures[0] = textures[1] = textures[2] = textures[3] = 0;
    }

    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
}

void PostProcessEffectChain::resize(QVector2D screenDimentions)
{
    // Find the largest potency-of-two dimention
    int size = 1;
    for (; size < screenDimentions.x(); size <<= 1) {}
    for (; size < screenDimentions.y(); size <<= 1) {}

    this->screenDimentions = screenDimentions;

    // And recreate the FBOs
    create(size);

    // And forward this new size to the effects
    for (auto &effect : activeEffects)
        effect->setScreenDimentions(screenDimentions);
}

void PostProcessEffectChain::addEffect(PostProcessEffect *effect)
{
    activeEffects.push_back(effect);
}

void PostProcessEffectChain::insert(unsigned int index, PostProcessEffect *effect)
{
    assert(index <= activeEffects.size() && "Index out of bounds");

    activeEffects.insert(activeEffects.begin() + index, effect);
}

void PostProcessEffectChain::clear()
{
    for (auto i = activeEffects.begin(); i != activeEffects.end(); ++i) {
        delete *i;
    }

    activeEffects.clear();
}

void PostProcessEffectChain::remove(int index)
{
    assert(index < activeEffects.size() && "Index out of bounds");

    auto i = activeEffects.begin() + index;

    delete *i;

    activeEffects.erase(i);
}

PostProcessEffect* PostProcessEffectChain::at(int index)
{
    assert(index < activeEffects.size() && "Index out of bounds");

    return *(activeEffects.begin() + index);
}

void PostProcessEffectChain::beginScene()
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFbo);

    glEnable(GL_DEPTH_TEST);

    // Set the viewport size as the FBO's size
    glViewport(0, 0, fboSize, fboSize);

    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        qWarning() << "Incomplete FBO; status " << status;

    GLenum attachment = GL_COLOR_ATTACHMENT0_EXT;
    glDrawBuffers(1, &attachment);

    status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        qWarning() << "Incomplete FBO; status " << status;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessEffectChain::endScene()
{
    glDisable(GL_DEPTH_TEST);
}

void PostProcessEffectChain::render()
{
    assert(fbo != 0 && "Effect chain FBO uninitialized");

    currentFbo = 0;

    GLenum attachment;

    attachment = GL_COLOR_ATTACHMENT1_EXT;
    glDrawBuffers(1, &attachment);
    final->render(textures[0], textures[0], textures[0]);

    // Iterate through all effects
    for (auto &effect : activeEffects) {
        // If the current effect is at an odd index, write to texture#1, else, to texture#2
        attachment = GL_COLOR_ATTACHMENT2_EXT - (currentFbo % 2);
        glDrawBuffers(1, &attachment);
        // Use the texture#0 as the original texture and, if the current effect is at an odd index, use texture #2 as input, else, use texture #1
        effect->render(textures[0], textures[currentFbo % 2 + 1], textures[3]);
        ++currentFbo;
    }

    // Reset our state
    attachment = GL_COLOR_ATTACHMENT0_EXT;
    glDrawBuffers(1, &attachment);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, oldFbo);

    glViewport(0, 0, screenDimentions.x(), screenDimentions.y());

    final->render(getOutputTexture(), getOutputTexture(), getOutputTexture());
}

GLuint PostProcessEffectChain::getOutputTexture()
{
    return textures[currentFbo % 2 + 1];
}

void PostProcessEffectChain::initializeGL()
{
    initializeOpenGLFunctions();

    final = new PassthroughEffect;
    final->create();

    // Add some effects to test
    EdgeDetectionEffect *edgeEffect = new EdgeDetectionEffect;
    edgeEffect->setAdditive(true);
    edgeEffect->setIntensity(0.2f);
    edgeEffect->create();
    addEffect(edgeEffect);

    /*GammaCorrectionEffect *gamma = new GammaCorrectionEffect;
    gamma->setGammaFactor(QVector3D(1.8f, 1.8f, 1.8f));
    gamma->create();
    addEffect(gamma);*/

    /*DepthOfFieldEffect *dofEffect = new DepthOfFieldEffect;
    dofEffect->create();
    addEffect(dofEffect);

    /*GaussianBlurEffect * blurEffect = new HorizontalGaussianBlurEffect;
    blurEffect->setSigma(2.5f);
    blurEffect->create();
    addEffect(blurEffect);

    blurEffect = new VerticalGaussianBlurEffect;
    blurEffect->setSigma(2.5f);
    blurEffect->create();
    addEffect(blurEffect);*/
}
