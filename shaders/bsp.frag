#version 400

in vec3 fN;
in vec3 fE;
in vec3 fL;
in vec2 fTexCoord;
in vec2 fLightmap;
in vec4 fColor;

out vec4 outColor;

uniform sampler2D albedoTexture;

void main(void)
{
    outColor = texture(albedoTexture, fTexCoord);
}
