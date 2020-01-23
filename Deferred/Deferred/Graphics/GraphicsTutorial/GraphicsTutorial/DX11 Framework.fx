//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

//Texture2D txDiffuse[2] : register(t0);
Texture2D TexArray[3] : register(t0);
Texture2D TexParallax : register(t1);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;
};

struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;
};

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
	float HasNormalMap;
    float HasParallaxMap;
	float UsesTangentSpace;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
};

struct VS_INPUT2
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT2
{
    float4 PosL : SV_POSITION;
    float3 NormL : NORMAL;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float3 Tangent : TANGENT;
    float3 BiTangent : BINORMAL;
	Light light : COLOR;

	float3 TangentviewPos : POSITION3;
	float3 TangentPixelPos : POSITION4;

};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);
	output.Tex = input.Tex;

	float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	output.NormW = normalize(normalW);

	output.Tangent = mul(float4(input.Tangent, 0.0f), World).xyz;
    output.Tangent = normalize(output.Tangent);

    output.BiTangent = mul(float4(input.Bitangent, 0.0f), World).xyz;
    output.BiTangent = normalize(output.BiTangent);

	float3 eyePos = EyePosW;

	eyePos = mul(float4(EyePosW, 0.0f), World).xyz;

    float3x3 TBN = transpose(float3x3(output.Tangent, output.BiTangent, output.NormW));

	output.TangentviewPos = mul(TBN, eyePos);
	output.TangentPixelPos = mul(TBN, output.PosH);
	output.NormW = normalize(mul(output.NormW, TBN));

	output.light.LightVecW = mul(light.LightVecW, TBN);


	return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float2 ParallaxMappingSteep(VS_OUTPUT input, float3 eye, float3 TanNormal)
{

	/*float3 viewDirW = -eye;

	float3 N = input.NormW;
	float3 T = normalize(input.Tangent - dot(input.Tangent, N) * N);
	float3 B = cross(N, T);

	float3x3 toTangent = transpose(float3x3(T, B, N));

	float3 viewDirTS = mul(viewDirW, toTangent);*/

	float minLayers = 64;
	float maxLayers = 128;
	//precompute texture gradients as cannot be done in loop
	float2 dx = ddx(input.Tex);
	float2 dy = ddy(input.Tex);

	float numLayers = lerp(minLayers, maxLayers, abs(dot(eye, TanNormal)));
	//float numLayers = mix(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), eye)));
	float layerDepth = 1.0f / numLayers;
	float currentLayerDepth = 0.0f;

	float2  height_scale = 0.04f;
	float2 p = eye.xy * height_scale;
	float2 deltaTexCoords = p / numLayers;

	float2 currentTexCoords = input.Tex;
	float currentDepthMapValue = TexArray[2].SampleGrad(samLinear, input.Tex, dx, dy).r;

	while (currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords += deltaTexCoords;
        currentDepthMapValue = TexArray[2].SampleGrad(samLinear, currentTexCoords, dx, dy).r;
		currentLayerDepth += layerDepth;
	}

	return currentTexCoords;
}

float2 ParallaxMapping(VS_OUTPUT input, float3 eye, float3 TanNormal)
{
    //float distToEye = length(eye);
    float2 finalTexOffset;
    int gMinSampleCount = 8;
    int gMaxSampleCount = 32;
    float gHeightScale = 0.04f;
	float3 viewDirTS = eye;
    float2 maxParallaxOffset = -viewDirTS.xy * gHeightScale / viewDirTS.z;

    int sampleCount = (int) lerp(gMaxSampleCount, gMinSampleCount, dot(eye, TanNormal));

    float zStep = 1.0f / (float) sampleCount;

    float2 texStep = maxParallaxOffset * zStep;

    //precompute texture gradients as cannot be done in loop
    float2 dx = ddx(input.Tex);
    float2 dy = ddy(input.Tex);

    int sampleIndex = 0;
    float2 currTexOffset = 0;
    float2 prevTexOffset = 0;
    float currRayZ = 1.0f - zStep;
    float prevRayZ = 1.0f;
    float currHeight = 0.0f;
    float prevHeight = 0.0f;
	float fCurrSampledHeight = 1.0f;
	float fLastSampledHeight = 1.0f;
	float fCurrRayHeight = 1.0f;

    //Ray trace the heightFeild
	while (sampleIndex < sampleCount)
	{
		currHeight = TexArray[2].SampleGrad(samLinear, input.Tex + currTexOffset, dx, dy).r;
		if (currHeight > currRayZ)
		{
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + zStep) - fLastSampledHeight;

			float ratio = delta1 / (delta1 + delta2);

			currTexOffset = (ratio)* prevTexOffset + (1.0 - ratio) * currTexOffset;

			sampleIndex = sampleCount + 1;
		}
		else
		{
			sampleIndex++;

			fCurrRayHeight -= zStep;

			prevTexOffset = currTexOffset;
			currTexOffset += zStep * maxParallaxOffset;

			fLastSampledHeight = fCurrSampledHeight;
		}
	}

    //Use these texture coordinates for all further textures
    float2 parallaxTex = input.Tex + currTexOffset;


	float height = TexArray[2].SampleGrad(samLinear, input.Tex , dx, dy).r;
	float2 p = eye.xy * (height * 0.02f);

	return input.Tex - p;



    //return parallaxTex;
}

float2 ParallaxOcclusionMapping(VS_OUTPUT input, float3 eye)
{
	float gHeightScale = 0.4f;
	int gMinSampleCount = 16;
	int gMaxSampleCount = 64;

	float3 viewDirW = -eye;

	float3 N = input.NormW;
	float3 T = normalize(input.Tangent - dot(input.Tangent, N) * N);
	float3 B = cross(N, T);

	float3x3 toTangent = transpose(float3x3(T, B, N));

	float3 viewDirTS = mul(viewDirW, toTangent);

	float2 maxParallaxOffset = -eye.xy * gHeightScale / eye.z;

	int sampleCount = (int)lerp(gMaxSampleCount, gMinSampleCount, dot(eye, input.NormW ));
	
	float zStep = 1.0f / (float)sampleCount;

	float2 texStep = maxParallaxOffset * zStep;

	float2 dx = ddx(input.Tex);
	float2 dy = ddy(input.Tex);

	int sampleIndex = 0;
	float2 currTexOffset = 0;
	float2 prevTexOffset = 0;
	float2 finalTexOffset = 0;
	float currRayZ = 1.0f;
	float prevRayZ = 1.0f;
	float currHeight = 0.0f;
	float prevHeight = 0.0f;

	while (sampleIndex < sampleCount + 1)
	{
		currHeight = TexArray[1].SampleGrad(samLinear, input.Tex + currTexOffset, dx, dy).a;

		if (currHeight > currRayZ)
		{
			float t = (prevHeight - prevRayZ) /
				(prevHeight - currHeight + currRayZ - prevRayZ);

			finalTexOffset = prevTexOffset + t * texStep;
			// Exit loop.
			sampleIndex = sampleCount + 1;
		}
		else
		{
			++sampleIndex;
			prevTexOffset = currTexOffset;
			prevRayZ = currRayZ;
			prevHeight = currHeight;
			currTexOffset += texStep;
			// Negative because we are going "deeper" into the surface.
			currRayZ -= zStep;
		}
	}

	float2 parallaxTex = input.Tex + finalTexOffset;

	return parallaxTex;
}


float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float4 tangentW)
{
	/*// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N);
	float3 B = tangentW.w * cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;*/
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 normalW = normalize(input.NormW);
	
	//float3 toEye = EyePosW - input.PosW;
	float3 toEye = normalize(input.TangentviewPos - input.TangentPixelPos);

	//float distToEye = length(toEye);

	//toEye /= distToEye;

	float2  texCoord = input.Tex;
    float3x3 texSpace;
	// Get texture data from file
	
	float3 tanNormal = normalW;

	
    if (HasParallaxMap == 1.0f)
    {
		//texCoord = ParallaxMapping(input, toEye, tanNormal);
		texCoord = ParallaxMappingSteep(input, toEye, tanNormal);
		//texCoord = ParallaxOcclusionMapping(input, toEye);
        //return texCoord.r;
		//texCoord = input.Tex;
    }
    else
    {
        texCoord = input.Tex;
    }

	if (texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
		discard;

  
    float4 textureColour = TexArray[0].Sample(samLinear, texCoord);

	if (UsesTangentSpace == 1.0f)
	{
		float4 normalMap = TexArray[1].Sample(samLinear, texCoord);
		//Make sure tangent is orthogonal to normal
		// input.Tangent = normalize(input.Tangent - dot(input.Tangent, tanNormal) * tanNormal);

		//create texture space
		//texSpace = float3x3(input.Tangent, input.BiTangent, tanNormal);
		//transpose(texSpace);

		//convert normal from normal map to texture space and store in normalW
		//tanNormal = normalize(mul(normalMap, texSpace));

		tanNormal = normalize(normalMap);
	}
	
       if (HasNormalMap == 1.0f)
        {
        float3 normalMap = TexArray[1].Sample(samLinear, texCoord);
            normalMap = (2.0f * normalMap) - 1.0f;
		//Make sure tangent is orthogonal to normal
          //  input.Tangent = normalize(input.Tangent - dot(input.Tangent, tanNormal) * tanNormal);
		//create texture space
          // float3x3 texSpace = float3x3(input.Tangent, input.BiTangent, tanNormal);

		//convert normal from normal map to texture space and store in normalW/TanNormal
		 tanNormal = normalize(normalMap);
		}

        float3 ambient = float3(0.0f, 0.0f, 0.0f);
        float3 diffuse = float3(0.0f, 0.0f, 0.0f);
        float3 specular = float3(0.0f, 0.0f, 0.0f);

        float3 lightLecNorm = normalize(input.light.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
    float3 r = reflect(-lightLecNorm, tanNormal);

	// Determine how much specular light makes it into the eye.
        float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
    float diffuseAmount = max(dot(lightLecNorm, tanNormal), 0.0f);

	// Only display specular when there is diffuse
        if (diffuseAmount <= 0.0f)
        {
            specularAmount = 0.0f;
        }

	// Compute the ambient, diffuse, and specular terms separately.
        specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
        diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
        ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

	// Sum all the terms together and copy over the diffuse alpha.
        float4 finalColour;
        if (HasTexture == 1.0f)
        {
            finalColour.rgb = ((textureColour.rgb * (ambient + diffuse)) + specular);
        }
       /* else if (HasNormalMap)
        {
		//finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
            finalColour.rgb = (textureColour.rgb * ((ambient + diffuse) + specular) * normalW);
        }*/
        else
        {
            finalColour.rgb = ambient + diffuse + specular;
        }

	
        finalColour.a = surface.DiffuseMtrl.a;

        return finalColour;
   // return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

VS_OUTPUT2 VS2(VS_INPUT2 input)
{
    VS_OUTPUT2 output;
    output.PosL = input.PosL;
    output.NormL = input.NormL;
    output.Tex = input.Tex;

    return output;
}

float4 PS2(VS_OUTPUT2 input) : SV_Target
{
    float4 normalMap = TexParallax.Sample(samLinear, input.Tex);
   // normalMap.r = normalMap.r * 0.0f;
    return normalMap;

}