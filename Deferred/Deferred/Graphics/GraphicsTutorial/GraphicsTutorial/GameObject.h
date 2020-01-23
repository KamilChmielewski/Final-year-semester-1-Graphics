#pragma once
#include <DirectXMath.h>
#include "Model.h"
#include "Struct.h"
#include "Material.h"
//Texture loading
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

class GameObject
{
private:
	bool hasNormalMap = false;
	bool hasParallaxMapping = false;
	Model* model;
	Material* material;
	
	XMFLOAT3 _scale;
	XMFLOAT3 _rotation;
	XMFLOAT4X4 _world;
	ID3D11Device* _pd3dDevice;

	ID3D11VertexShader* _pVertexShader;
	ID3D11PixelShader* _pPixelShader;

	ID3D11ShaderResourceView* _pTextureRV;
	ID3D11ShaderResourceView* _pTextureArray[3];
	ID3D11ShaderResourceView* _pTextureHeightMap;
	ID3D11SamplerState* _pSamplerLinear = nullptr;

public:

	XMFLOAT3 _position;
	GameObject(XMFLOAT3 _position, Model* model, ID3D11Device* _pd3dDevice, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader);
	~GameObject();
	XMMATRIX GetMatrix() { return XMMatrixIdentity() * XMMatrixTranslation(_position.x, _position.y, _position.z); }
	
	void SetTextureRV(ID3D11ShaderResourceView* textureRV) { _pTextureRV = textureRV; }
	void SetParallaxMapping(bool state) { hasParallaxMapping = state; }
	void SetHasNormalMap(bool normalMap);
	bool HasNormalMap() const { return hasNormalMap ? true : false; }
	bool HasTexture() const { return _pTextureRV ? true : false; }
	bool HasParallaxMapping() { return hasParallaxMapping ? true : false; }

	void CalculateModelVectors();
	void CalculatetangentBinormal2(Vertex v1, Vertex v2, Vertex v3, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal);
	ID3D11ShaderResourceView* GetTextureRV() const { return _pTextureRV; }
	ID3D11ShaderResourceView** GetTextureArray() { return _pTextureArray; }
	ID3D11ShaderResourceView* GetTextureHeightMap() { return _pTextureHeightMap; }
	void CreateAndSetSamplerState();
	
	void CreateAndSetTextureHeightWic(const wchar_t* filePath);
	void CreateAndSetTextureWIC(const wchar_t* filePath);
	void CreateAndSetTextureDDS(const wchar_t* filePath);
	void CreateAndSetTextureArrayWIC(const wchar_t* filePath, const wchar_t* filePath2);
	//void CreateAndSetTextureArrayDDS(const wchar_t* filePath, const wchar_t* filePath2);
	//void CreateAndSetTextureArrayWiC_AND_DSS(const wchar_t* filePath, const wchar_t* filePath2);

	void SetScale(XMFLOAT3 _scale);
	void SetRotation(XMFLOAT3 _rotation);
	void AddRotation(XMFLOAT3 _rotation) { this->_rotation.x += _rotation.x; this->_rotation.y += _rotation.y; this->_rotation.z += _rotation.z;}
	void SetTranslation(XMFLOAT3 _postion);
	void Update(float t);
	
	void DrawDeferred(ID3D11Buffer* _pConstantBuffer, ID3D11DeviceContext* _pImmediateContext, DeferredConstantBuffer& cb, XMMATRIX result);
	void Draw(ID3D11Buffer* _pConstantBuffer, ID3D11DeviceContext* _pImmediateContext, ConstantBuffer& cb, XMMATRIX result);
	

	void ModelToLoad(Model* model);
};