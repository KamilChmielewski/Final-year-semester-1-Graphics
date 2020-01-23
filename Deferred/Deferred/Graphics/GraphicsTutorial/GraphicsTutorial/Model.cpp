#include "Model.h"

Model::Model(ID3D11Device* _pd3dDevice)
{
	mesh = nullptr;
	this->_pd3dDevice = _pd3dDevice;
}

Model::~Model()
{

}

void Model::LoadModel(const char* path)
{
	mesh = ModelLoader::Load(path, _pd3dDevice);
}

void Model::Draw(ID3D11DeviceContext* _pImmediateContext)
{
	if (mesh == nullptr)
	{
		return;
	}
	static UINT stride = sizeof(Vertex);
	static UINT offset = 0;

	_pImmediateContext->IASetVertexBuffers(0, 1, &mesh->_pVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(mesh->_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->DrawIndexed(mesh->Indices.size(), 0, 0);
}