#version 400

// http://xissburg.com/pt-br/desfoque-gaussiano-mais-rapido-em-glsl/

in vec4 vPosition;
in vec2 vTexCoord;

uniform vec2 texelSize;

out vec2 texCoord;
out vec2 blurTexCoords[14];

void main(void)
{
    gl_Position = vPosition;
    texCoord = vTexCoord;
    blurTexCoords[ 0] = texCoord + vec2(texelSize.x * -7, 0.0);
    blurTexCoords[ 1] = texCoord + vec2(texelSize.x * -6, 0.0);
    blurTexCoords[ 2] = texCoord + vec2(texelSize.x * -5, 0.0);
    blurTexCoords[ 3] = texCoord + vec2(texelSize.x * -4, 0.0);
    blurTexCoords[ 4] = texCoord + vec2(texelSize.x * -3, 0.0);
    blurTexCoords[ 5] = texCoord + vec2(texelSize.x * -2, 0.0);
    blurTexCoords[ 6] = texCoord + vec2(texelSize.x * -1, 0.0);
    blurTexCoords[ 7] = texCoord + vec2(texelSize.x *  1, 0.0);
    blurTexCoords[ 8] = texCoord + vec2(texelSize.x *  2, 0.0);
    blurTexCoords[ 9] = texCoord + vec2(texelSize.x *  3, 0.0);
    blurTexCoords[10] = texCoord + vec2(texelSize.x *  4, 0.0);
    blurTexCoords[11] = texCoord + vec2(texelSize.x *  5, 0.0);
    blurTexCoords[12] = texCoord + vec2(texelSize.x *  6, 0.0);
    blurTexCoords[13] = texCoord + vec2(texelSize.x *  7, 0.0);
}

