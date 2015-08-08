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
uniform vec3 lightColor;
uniform float lightIntensity;

void main(void)
{
    vec3 N = normalize(fN);
    vec3 E = normalize(fE);
    vec3 L = normalize(fL);
    vec3 R = normalize(2.0 * dot(L, N) * N - L);

    float NdotL = dot(N, L);
    float Kd = max(NdotL, 0.0);
    float Ks = (NdotL < 0.0) ? 0.0 : pow(max(dot(R, E), 0.0), lightIntensity);

    vec4 diffuse = Kd * texture(albedoTexture, fTexCoord);
    vec3 ambient = lightColor * texture(lightmapTexture, fLightmap).rgb;
    vec4 specular = Ks * vec4(lightColor, 1.0);

    outColor = vec4(ambient + diffuse.rgb + specular.rgb, diffuse.a);
}
