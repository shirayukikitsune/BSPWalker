#version 400

in vec3 fN;
in vec3 fE;
in vec3 fL;
in vec2 fTexCoord;
in vec2 fLightmap;
in vec4 fColor;

out vec4 outColor;

uniform sampler2D albedoTexture;
uniform sampler2D lightmapTexture;

void main(void)
{
    vec3 lightColor = texture(lightmapTexture, fLightmap).rgb;
    vec4 albedoColor = texture(albedoTexture, fTexCoord);

    outColor.a = albedoColor.a;
    outColor.rgb = albedoColor.rgb * lightColor.rgb;
}
