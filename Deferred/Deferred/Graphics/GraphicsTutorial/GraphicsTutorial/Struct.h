#pragma once
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3d11_1.h>
#include <vector>

using namespace DirectX;

struct Textures
{
	float u, v;
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 UV;
	XMFLOAT3 Tangent;
	XMFLOAT3 Bitangent;
};

struct Mesh
{
	std::vector<Vertex> elements;
	std::vector<unsigned short> Indices;
	unsigned short vertexIndex[3], uvIndex[3], normalIndex[3];
	std::vector<unsigned short> vertexIndices, uvIndices, normalIndices;
	std::vector<XMFLOAT3> indexed_vertices;
	std::vector<XMFLOAT2> indexed_uvs;
	std::vector<XMFLOAT3> indexed_normals;

	std::vector<XMFLOAT3> indexed_tangents;
	std::vector<XMFLOAT3> indexed_binoramls;

	ID3D11Buffer* _pVertexBuffer;
	ID3D11Buffer* _pIndexBuffer;
};

struct PackedVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 UV;
	//Overloading < function
	bool operator <(const PackedVertex that) const { return memcmp((void*)this, (void*)& that, sizeof(PackedVertex)) > 0; } //So if that is greater than this then return true
};

struct DirectionalLight
{
	//	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad; // Not sure if I need this but I can now have an array of lights if I wanted to?
};

struct MaterialStruct
{
	//	Material() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT4 Reflect;
};


struct SurfaceInfo
{
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 SpecularMtrl;
};

struct Light
{
	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;

	float SpecularPower;
	XMFLOAT3 LightVecW;
};

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	SurfaceInfo surface;

	Light light;

	XMFLOAT3 EyePosW;
	float HasTexture;
	float HasNormalMap;
	float HasParallaxMap;
	float UsesTangentSpace;
};

struct DeferredConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	XMFLOAT3 EyePosW;
};

struct DeferredLightConstantBuffer
{
	Light light;
	//float padding1;
	//float padding2;
	//float padding3;
};

struct DissolveConstantBuffer
{
	float time = 0.0f;
	float start_time = 99999999.0f;
	float padding;
	float padding2;
};

struct SimpleVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
};