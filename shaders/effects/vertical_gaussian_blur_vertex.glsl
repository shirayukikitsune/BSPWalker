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
    blurTexCoords[ 0] = texCoord + vec2(0.0, texelSize.y * -7);
    blurTexCoords[ 1] = texCoord + vec2(0.0, texelSize.y * -6);
    blurTexCoords[ 2] = texCoord + vec2(0.0, texelSize.y * -5);
    blurTexCoords[ 3] = texCoord + vec2(0.0, texelSize.y * -4);
    blurTexCoords[ 4] = texCoord + vec2(0.0, texelSize.y * -3);
    blurTexCoords[ 5] = texCoord + vec2(0.0, texelSize.y * -2);
    blurTexCoords[ 6] = texCoord + vec2(0.0, texelSize.y * -1);
    blurTexCoords[ 7] = texCoord + vec2(0.0, texelSize.y *  1);
    blurTexCoords[ 8] = texCoord + vec2(0.0, texelSize.y *  2);
    blurTexCoords[ 9] = texCoord + vec2(0.0, texelSize.y *  3);
    blurTexCoords[10] = texCoord + vec2(0.0, texelSize.y *  4);
    blurTexCoords[11] = texCoord + vec2(0.0, texelSize.y *  5);
    blurTexCoords[12] = texCoord + vec2(0.0, texelSize.y *  6);
    blurTexCoords[13] = texCoord + vec2(0.0, texelSize.y *  7);
}
