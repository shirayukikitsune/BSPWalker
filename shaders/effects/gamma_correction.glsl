#version 400

uniform sampler2D originalTexture;
uniform sampler2D chainedTexture;

uniform vec3 gammaFactor;

in vec2 texCoord;
out vec4 myfragcolor;

void main(void)
{
    vec3 color = texture2D(chainedTexture, texCoord).rgb;

    myfragcolor.rgb = pow(color, 1.0 / gammaFactor);
    myfragcolor.a = 1.0;
}

