#version 400

in vec3 vPosition;
in vec2 vTexCoord;
in vec2 vLightmapCoord;
in vec3 vNormal;
in vec4 vColor;

out vec3 fN;
out vec3 fE;
out vec3 fL;
out vec2 fTexCoord;
out vec2 fLightmap;
out vec4 fColor;

uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightDirection;

void main(void)
{
    vec4 eyePosition = modelView * vec4(vPosition, 1.0);

    fN = normalMatrix * vNormal;
    fL = lightDirection;
    fE = -eyePosition.xyz;
    fTexCoord = vTexCoord;
    fLightmap = vLightmapCoord;
    fColor = vColor;

    gl_Position = projectionMatrix * eyePosition;
}

