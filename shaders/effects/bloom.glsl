#version 400

uniform sampler2D originalTexture;
uniform sampler2D chainedTexture;

uniform float bloomIntensity;
uniform float bloomSaturation;
uniform float originalIntensity;
uniform float originalSaturation;

in vec2 texCoord;
out vec4 myfragcolor;

vec4 adjustSaturation(vec4 color, float saturation)
{
    return mix(vec4(dot(color, vec4(0.2, 0.2, 0.2, 1.0))), color, vec4(saturation));
}

void main(void)
{
    vec4 bloomColor = texture2D(chainedTexture, texCoord);
    vec4 originalColor = texture2D(originalTexture, texCoord);

    bloomColor = adjustSaturation(bloomColor, bloomSaturation) * bloomIntensity;
    originalColor = adjustSaturation(originalColor, originalSaturation) * originalIntensity;

    originalColor = originalColor * (vec4(1.0) - clamp(bloomColor, 0.0, 1.0));

    myfragcolor = originalColor + bloomColor;
}

