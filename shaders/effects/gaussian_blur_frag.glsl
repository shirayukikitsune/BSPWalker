#version 400

// http://xissburg.com/pt-br/desfoque-gaussiano-mais-rapido-em-glsl/

uniform sampler2D originalTexture;
uniform sampler2D chainedTexture;
uniform float gaussianWeights[15];

in vec2 blurTexCoords[14];
in vec2 texCoord;

out vec4 myfragcolor;

void main()
{
    myfragcolor = vec4(0.0);
    /*myfragcolor += texture2D(chainedTexture, blurTexCoords[ 0])*0.0044299121055113265;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 1])*0.00895781211794;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 2])*0.0215963866053;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 3])*0.0443683338718;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 4])*0.0776744219933;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 5])*0.115876621105;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 6])*0.147308056121;
    myfragcolor += texture2D(chainedTexture, texCoord         )*0.159576912161;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 7])*0.147308056121;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 8])*0.115876621105;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 9])*0.0776744219933;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[10])*0.0443683338718;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[11])*0.0215963866053;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[12])*0.00895781211794;
    myfragcolor += texture2D(chainedTexture, blurTexCoords[13])*0.0044299121055113265;*/
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 0])*gaussianWeights[0];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 1])*gaussianWeights[1];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 2])*gaussianWeights[2];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 3])*gaussianWeights[3];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 4])*gaussianWeights[4];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 5])*gaussianWeights[5];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 6])*gaussianWeights[6];
    myfragcolor += texture2D(chainedTexture, texCoord         )*gaussianWeights[7];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 7])*gaussianWeights[8];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 8])*gaussianWeights[9];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[ 9])*gaussianWeights[10];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[10])*gaussianWeights[11];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[11])*gaussianWeights[12];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[12])*gaussianWeights[13];
    myfragcolor += texture2D(chainedTexture, blurTexCoords[13])*gaussianWeights[14];
}
