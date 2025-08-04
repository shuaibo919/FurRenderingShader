#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_noise;
uniform vec3 viewPos;

const int SampleCount = 64; // Number of fur samples
const float FurLength = 1.5f; // Length of the fur

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    gPosition = vec3(1);
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // And the diffuse per-fragment color

    vec4 ResultColor = vec4(0,0,0,0);
    float ShouldContinue = 1.0;

    vec3 ViewDir = normalize(FragPos - viewPos);
    vec3 TagenPixelToCamera = TBN * ViewDir;
    vec2 UVOffset = FurLength * TagenPixelToCamera.xy;

    for(int i = 0; i < SampleCount + 1; ++i)
    {

        // 计算Layer
        float CurLayer = float(SampleCount - i)/float(SampleCount);
        vec2 CurUVOffset = -UVOffset * CurLayer * FurLength;

        // UV矫正
        vec2 CurUV = TexCoords + 0.04 * CurUVOffset ;
        vec2 PatternUV = CurUV * 15;
        vec2 CurPatternUV = PatternUV + 0.08 * CurUVOffset;

        // FurPattern控制，当前Layer大于Pattern的采样值才计算贡献, 可用的函数: x, x^2, sqrt(x)....
        float Alpha = texture(texture_noise, CurPatternUV).r;
        float PatternMask =  step(CurLayer * CurLayer, Alpha);
        
        // 越靠外的毛发计算叠加颜色时的透明度越高，  可用的函数: 1-x, 1-x^2, 1-sqrt(x)...
        Alpha = (1 - CurLayer * CurLayer);

        // 采样BaseColor
        vec4 BaseColor =  texture(texture_diffuse, CurUV);
        BaseColor.a *= Alpha;
        BaseColor.rgb -= (pow(1.0 - CurLayer, 3)) * 0.04;

        // 累计Color
        float Remain = (1. - ResultColor.a);
        ResultColor += vec4(BaseColor.rgb * Remain * BaseColor.a, BaseColor.a) * PatternMask  * ShouldContinue;
        ResultColor = clamp(ResultColor, 0, 1);
        
        // ResultColor = 1.0时，结束叠加
        ShouldContinue *= step(ResultColor.a, 0.9999);
    }

    gAlbedoSpec = ResultColor;
}