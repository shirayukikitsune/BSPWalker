#version 400

// http://www.geeks3d.com/20110408/cross-stitching-post-processing-shader-glsl-filter-geexlab-pixel-bender/

uniform sampler2D originalTexture;
uniform sampler2D chainedTexture;

uniform vec2 screenDimentions;
uniform vec2 texelSize;
uniform float stitchSize;
uniform int inverted;

in vec2 texCoord;
out vec4 myfragcolor;

void main(void) {
    vec2 cPos = texCoord * screenDimentions;
    vec2 tlPos = floor(cPos / vec2(stitchSize, stitchSize));
    tlPos *= stitchSize;
    int remX = int(mod(cPos.x, stitchSize));
    int remY = int(mod(cPos.y, stitchSize));
    if (remX == 0 && remY == 0) tlPos = cPos;
    vec2 blPos = tlPos;
    blPos.y += (stitchSize - 1.0f);
    if (remX == remY || int(cPos.x) - int(blPos.x) == int(blPos.y) - int(cPos.y)) {
        if (inverted == 1) {
           myfragcolor = vec4(0.2, 0.15, 0.05, 1.0);
        }
        else {
           myfragcolor = texture2D(chainedTexture, tlPos * texelSize) * 1.4f;
        }
    }
    else {
        if (inverted == 1) {
            myfragcolor = texture2D(chainedTexture, tlPos * texelSize) * 1.4f;
        }
        else {
            myfragcolor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}
