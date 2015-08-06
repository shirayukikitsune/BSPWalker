#version 400

uniform sampler2D originalTexture;
uniform sampler2D chainedTexture;

uniform vec2 screenDimentions;
uniform vec2 texelSize;
uniform float intensity;
uniform bool addOriginal;

in vec2 texCoord;
out vec4 myfragcolor;

// Constantes pré-definidas pelo algoritmo Frei-Chen
const mat3 G[] = mat3[9](
    1.0 / (2.0*sqrt(2.0)) * mat3(1.0, sqrt(2.0), 1.0, 0.0, 0.0, 0.0, -1.0, -sqrt(2.0), -1.0),
    1.0 / (2.0*sqrt(2.0)) * mat3(1.0, 0.0, -1.0, sqrt(2.0), 0.0, -sqrt(2.0), 1.0, 0.0, -1.0),
    1.0 / (2.0*sqrt(2.0)) * mat3(0.0, -1.0, sqrt(2.0), 1.0, 0.0, -1.0, -sqrt(2.0), 1.0, 0.0),
    1.0 / (2.0*sqrt(2.0)) * mat3(sqrt(2.0), -1.0, 0.0, -1.0, 0.0, 1.0, 0.0, 1.0, -sqrt(2.0)),
    1.0 / 2.0 * mat3(0.0, 1.0, 0.0, -1.0, 0.0, -1.0, 0.0, 1.0, 0.0),
    1.0 / 2.0 * mat3(-1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0),
    1.0 / 6.0 * mat3(1.0, -2.0, 1.0, -2.0, 4.0, -2.0, 1.0, -2.0, 1.0),
    1.0 / 6.0 * mat3(-2.0, 1.0, -2.0, 1.0, 4.0, 1.0, -2.0, 1.0, -2.0),
    1.0 / 3.0 * mat3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)
);

void main(void)
{
    int i, j;
    mat3 I;
    vec3 color;
    float cnv[9];

    // Pega a cor dos pontos ao redor do ponto atual (todos os pontos adjacentes, incluindo os diagonais), utiliza um vetor
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            color = texture2D(originalTexture, (texCoord * screenDimentions + vec2(i - 1, j - 1)) * texelSize).rgb;
            I[i][j] = length(color);
        }
    }

    for (i = 0; i < 9; i++) {
        mat3 G2 = G[i];
        float dp3 = dot(G2[0], I[0]) + dot(G2[1], I[1]) + dot(G2[2], I[2]);
        cnv[i] = dp3 * dp3;
    }

    float M = (cnv[0] + cnv[1]) + (cnv[2] + cnv[3]);
    float S = (cnv[4] + cnv[5]) + (cnv[6] + cnv[7]) + cnv[8] + M;

    myfragcolor = addOriginal ?
                texture2D(chainedTexture, texCoord) - vec4(vec3(sqrt(M / S) * intensity), 0.0) :
                vec4(vec3(sqrt(M / S) * intensity), texture2D(originalTexture, texCoord).a);
}

