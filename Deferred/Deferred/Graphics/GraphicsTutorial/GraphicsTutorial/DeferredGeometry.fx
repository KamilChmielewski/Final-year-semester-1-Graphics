//An RGB diffuse color vector also known as albedo.

Texture2D Texure[3] : register(t0);

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_POINT;

    AddressU = Wrap;

    AddressV = Wrap;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;

    float3 EyePosW;
}

struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 NormW : NORMAL;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    
    float3 Tangent : TANGENT;
    float3 BiTangent : BINORMAL;

    float3 TangentviewPos : POSITION3;
    float3 TangentPixelPos : POSITION4;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 posW = mul(input.PosL, World);
    output.PosW = posW.xyz;

    output.PosH = mul(posW, View);
    output.PosH = mul(output.PosH, Projection);
    
    float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
    output.NormW = normalize(normalW);
    output.Tex = input.Tex;
    
    /*output.Tangent = mul(float4(input.Tangent, 0.0f), World).xyz;
    output.Tangent = normalize(output.Tangent);

    output.BiTangent = mul(float4(input.Bitangent, 0.0f), World).xyz;
    output.BiTangent = normalize(output.BiTangent);

    float3 eyePos = EyePosW;

    eyePos = mul(float4(EyePosW, 0.0f), World).xyz;

    float3x3 TBN = transpose(float3x3(output.Tangent, output.BiTangent, output.NormW));

    output.TangentviewPos = mul(TBN, eyePos);
    output.TangentPixelPos = mul(TBN, output.PosH);
    output.NormW = normalize(mul(output.NormW, TBN));*/
    
    return output;
}


float2 ParallaxMappingSteep(VS_OUTPUT input, float3 eye, float3 TanNormal)
{
    float minLayers = 64;
    float maxLayers = 128;
	//precompute texture gradients as cannot be done in loop
    float2 dx = ddx(input.Tex);
    float2 dy = ddy(input.Tex);

    float numLayers = lerp(minLayers, maxLayers, abs(dot(eye, TanNormal)));
	//float numLayers = mix(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), eye)));
    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0f;

    float2 height_scale = 0.04f;
    float2 p = eye.xy * height_scale;
    float2 deltaTexCoords = p / numLayers;

    float2 currentTexCoords = input.Tex;
    float currentDepthMapValue = Texure[2].SampleGrad(samLinear, input.Tex, dx, dy).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords += deltaTexCoords;
        currentDepthMapValue = Texure[2].SampleGrad(samLinear, currentTexCoords, dx, dy).r;
        currentLayerDepth += layerDepth;
    }

    return currentTexCoords;
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float4 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N);
	float3 B = tangentW.w * cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

PS_OUTPUT PS(VS_OUTPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    
    float3 normalW = normalize(input.NormW);
	
    float3 toEye = normalize(input.TangentviewPos - input.TangentPixelPos);

    float2 texCoord = input.Tex;
    
    float3 tanNormal = normalW;
    
    texCoord = ParallaxMappingSteep(input, toEye, tanNormal);
    
    float3 normalMap = Texure[1].Sample(samLinear, texCoord);

    tanNormal = normalize(normalMap);
    
    normalMap = (2.0f * normalMap) - 1.0f;
    tanNormal = normalize(normalMap);
    
    float3 temp = mul(float4(tanNormal, 0.0f), transpose(World)).xyz;

	//sample the color from the texture and store it for the output to the render target
    output.color = Texure[0].Sample(samLinear, input.Tex);

	//store the normal for output to the render target
    output.normal = float4(input.NormW, 1.0f);

    return output;
}

