
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);


SamplerState samLinear
{
	Filter = MIN_MAG_MIP_POINT;

	AddressU = Wrap;

	AddressV = Wrap;
};


struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;
};

cbuffer LightBuffer : register(b0)
{
	Light light;
//	float padding1;
//	float padding2;
// 	float padding3;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 NormL : NORMAL;
	float2 tex : TEXCOORD0;
};

PS_OUTPUT VS(VS_INPUT input)
{
	PS_OUTPUT output;
	output.position = input.PosL;
	output.NormL = input.NormL;
	output.tex = input.Tex;

	return output;
}

float4 PS(PS_OUTPUT input) : SV_TARGET
{
	float4 colour;
	float4 normals;
	float3 lightDir;
	float3 ConstantLight = light.LightVecW;
	float lightIntensity;
	float4 finalColour;

	colour = colorTexture.Sample(samLinear, input.tex);

    normals = normalTexture.Sample(samLinear, input.tex);

	lightDir = normalize(ConstantLight);

	lightDir = lightDir;

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);


	lightIntensity = saturate(dot(normals.xyz, lightDir));

	finalColour = saturate(colour * lightIntensity);


	return finalColour;
}