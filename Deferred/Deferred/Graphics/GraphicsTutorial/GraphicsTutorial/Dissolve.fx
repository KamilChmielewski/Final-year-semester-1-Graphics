
Texture2D Texure : register(t0);
Texture2D TexHeightMap : register(t1);
Texture2D TexGradiant : register(t2);
Texture2D TexBeginTex : register(t3);

//float start_time = 99999999.0f;
float duration = 10.0f;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_POINT;

    AddressU = Wrap;

    AddressV = Wrap;
};

struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 PosL : SV_POSITION;
    float3 NormL : NORMAL;
    float2 Tex : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
    float time;
    float start_time;
    float padding;
    float padding2;
}

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    output.PosL = input.PosL;
    output.NormL = input.NormL;
    output.Tex = input.Tex;

    return output;
}

float4 burnmagic(float4 begin_texture, float4 end_texture, float time)
{
    //if (time <= 0.0f)
    //    return begin_texture;
    
    //if (time >= 1.0f)
    //    return end_texture;

    //float4 c = tex2D(_MainTex, i.uv);
    //float val = 1 - tex2D(_DissolveTex, i.uv).r;
    //if (val < _Threshold - 0.04)
    //{
    //    discard;
    //}
 
    //bool b = val < _Threshold;
    //return lerp(c, c * fixed4(lerp(1, 0, 1 - saturate(abs(_Threshold - val) / 0.04)), 0, 0, 1), b);

    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 PS(VS_OUTPUT input) : SV_Target
{
    //input.Tex.x = input.Tex.x / 0.5;
   // input.Tex.x = sin(0.2);
    float4 normalTexture = Texure.Sample(samLinear, input.Tex);
    float4 TexHeightMapTexture = TexHeightMap.Sample(samLinear, input.Tex);
    float4 BrunTexture = TexGradiant.Sample(samLinear, input.Tex);
    float4 StartTex = TexBeginTex.Sample(samLinear, input.Tex);

    float2 dx = ddx(input.Tex);
    float2 dy = ddy(input.Tex);
    float TexHeightMapTextureNoise = TexHeightMap.SampleGrad(samLinear, input.Tex, dx, dy).r;
    float Burn = TexGradiant.SampleGrad(samLinear, input.Tex, dx, dy).rgb * -1;
  
    if (TexHeightMapTextureNoise < time / 10.f)
    {
       // TexHeightMapTexture.r = 1.0f;
		//normalTexture.r = 1.0f;//TexHeightMapTexture.r * normalTexture.r * BrunTexture.r;
       // normalTexture.b = TexHeightMapTexture.r * normalTexture.b * BrunTexture.b;
       // normalTexture.g = TexHeightMapTexture.r * normalTexture.g * BrunTexture.g;
		//TexHeightMapTextureNoise = 0.7f;
        StartTex = /*StartTex **/ normalTexture  /* TexHeightMapTextureNoise*/;
       /// StartTex.rbg = Burn;

    }


    
	//StartTex.r = sin(time);
	// StartTex.b = cos(time);
    return StartTex /* TexHeightMapTexture*/;
   // return normalTexture  /** TexHeightMapTexture*/;


}

