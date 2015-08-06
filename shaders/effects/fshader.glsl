#version 400

uniform sampler2D chainedTexture;

in vec2 texCoord;

out vec4 myfragcolor;

void main(void)
{
    myfragcolor = texture2D(chainedTexture, texCoord);
}
