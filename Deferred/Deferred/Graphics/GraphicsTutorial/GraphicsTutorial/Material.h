#pragma once
#include <DirectXMath.h>
#include "Struct.h"

using namespace DirectX;

class Material
{
private:
	ID3D11VertexShader* _pVertexShader;
	ID3D11PixelShader* _pPixelShader;
	ID3D11ShaderResourceView* _pTextureRV;

	//Struct Used
	MaterialStruct Mtrl;

public:
	Material(MaterialStruct Mtrl, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader, ID3D11ShaderResourceView* _pTextureRV);
	Material(MaterialStruct Mtrl, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader);
	~Material();

	void Draw(ID3D11DeviceContext* _pImmediateContext, ConstantBuffer& cb);

};