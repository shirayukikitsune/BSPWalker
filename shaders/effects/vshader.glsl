#version 400

in vec4 vPosition;
in vec2 vTexCoord;

out vec2 texCoord;

void main()
{
    texCoord = vTexCoord;
    gl_Position = vPosition;
}
