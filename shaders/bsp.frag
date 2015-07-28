#version 400

struct FragmentInfo {
    vec3 normal;
    vec3 eyeDirection;
    vec3 lightDirection;
    vec2 texCoord;
    vec2 lightmapCoord;
    vec4 color;
};

in FragmentInfo fData;

out vec4 fColor;

void main(void)
{
    fColor = vec4(1.0);
}
