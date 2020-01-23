#include "Material.h"

Material::Material(MaterialStruct Mtrl, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader, ID3D11ShaderResourceView* _pTextureRV)
{
	this->Mtrl = Mtrl;
	this->_pVertexShader = _pVertexShader;
	this->_pPixelShader = _pPixelShader;
	this->_pTextureRV = _pTextureRV;
}

Material::Material(MaterialStruct Mtrl, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader)
{
	this->Mtrl = Mtrl;
	this->_pVertexShader = _pVertexShader;
	this->_pPixelShader = _pPixelShader;
	this->_pTextureRV = nullptr;
}

Material::~Material()
{
}

void Material::Draw(ID3D11DeviceContext* _pImmediateContext, ConstantBuffer& cb)
{
	//cb.Mtrl = Mtrl;

	if (_pTextureRV)
	{
		_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
	}

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
}
