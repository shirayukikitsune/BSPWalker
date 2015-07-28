#version 400

in vec4 vPosition;
in vec2 vTexCoord;
in vec2 vLightmapCoord;
in vec4 vNormal;
in vec4 vColor;

struct FragmentInfo {
    vec2 texCoord;
    vec2 lightmapCoord;
    vec4 color;
};

out FragmentInfo fData;

void main(void)
{
    fData.texCoord = vTexCoord;
    fData.lightmapCoord= vLightmapCoord;
    fData.color = vColor;

    gl_Position = vPosition;
}

