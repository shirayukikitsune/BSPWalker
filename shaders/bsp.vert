#version 400

in vec4 vPosition;
in vec2 vTexCoord;
in vec2 vLightmapCoord;
in vec4 vNormal;
in vec4 vColor;

struct FragmentInfo {
    vec3 normal;
    vec3 eyeDirection;
    vec3 lightDirection;
    vec2 texCoord;
    vec2 lightmapCoord;
    vec4 color;
};

out FragmentInfo fData;

uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightPosition;

void main(void)
{
    vec4 eyePosition = modelView * vPosition;

    fData.normal = normalMatrix * vNormal.xyz;
    fData.lightDirection = lightPosition.xyz - eyePosition.xyz;
    fData.eyeDirection = -eyePosition.xyz;
    fData.texCoord = vTexCoord;
    fData.lightmapCoord= vLightmapCoord;
    fData.color = vColor;

    gl_Position = projectionMatrix * eyePosition;
}

