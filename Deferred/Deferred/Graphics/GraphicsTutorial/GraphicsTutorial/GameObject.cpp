#include "GameObject.h"
GameObject::GameObject(XMFLOAT3 _position, Model* model, ID3D11Device* _pd3dDevice, ID3D11VertexShader* _pVertexShader, ID3D11PixelShader* _pPixelShader)
{
	this->_position = _position;
	this->_pd3dDevice = _pd3dDevice;
	this->_pVertexShader = _pVertexShader;
	this->_pPixelShader = _pPixelShader;
	this->model = model;


	//default Matrix values;
	_scale.x = 0.5f, _scale.y = 0.5, _scale.z = 0.5f;
	_rotation.x = 0.0f, _rotation.y = 0.0, _rotation.z = 0.0f;
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

	MaterialStruct Mtrl;

	Mtrl.Ambient = XMFLOAT4(0.0f, 1.0f, 0.2f, 1.0f);
	Mtrl.Diffuse = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	Mtrl.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	material = new Material(Mtrl, this->_pVertexShader, this->_pPixelShader);

}

GameObject::~GameObject()
{

}

void GameObject::SetHasNormalMap(bool normalMap)
{
	hasNormalMap = normalMap;

	if (hasNormalMap)
	{

	}
		//CalculateModelVectors();
}

void GameObject::CalculatetangentBinormal2(Vertex v0, Vertex v1, Vertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal)
{
	XMFLOAT3 edge1(v1.Pos.x - v0.Pos.x, v1.Pos.y - v0.Pos.y, v1.Pos.z - v0.Pos.z);
	XMFLOAT3 edge2(v2.Pos.x - v0.Pos.x, v2.Pos.y - v0.Pos.y, v2.Pos.z - v0.Pos.z);

	XMFLOAT2 deltaUV1(v1.UV.x - v0.UV.x, v1.UV.y - v0.UV.y);
	XMFLOAT2 deltaUV2(v2.UV.x - v0.UV.x, v2.UV.y - v0.UV.y);

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	XMVECTOR tn = XMLoadFloat3(&tangent);
	tn = XMVector3Normalize(tn);
	XMStoreFloat3(&tangent, tn);

	binormal.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	binormal.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	binormal.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	tn = XMLoadFloat3(&binormal);
	tn = XMVector3Normalize(tn);
	XMStoreFloat3(&binormal, tn);


	XMVECTOR vv0 = XMLoadFloat3(&v0.Pos);
	XMVECTOR vv1 = XMLoadFloat3(&v1.Pos);
	XMVECTOR vv2 = XMLoadFloat3(&v2.Pos);

	XMVECTOR e0 = vv1 - vv0;
	XMVECTOR e1 = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross(e0, e1);
	e01cross = XMVector3Normalize(e01cross);
	XMFLOAT3 normalOut;
	XMStoreFloat3(&normalOut, e01cross);
	normal = normalOut;

	return;
}

void GameObject::CalculateModelVectors()
{
	Vertex vertex1, vertex2, vertex3;
	XMFLOAT3 tangent, binormal, normal;
	int facecount = model->GetMesh()->elements.size() /3;
	int index = 0;
	
	for (int i = 0; i < facecount; i++)
	{
		std::cout << "Index: " << index << std::endl;
		vertex1.Pos.x	 = model->GetMesh()->elements[index].Pos.x;
		vertex1.Pos.y 	 = model->GetMesh()->elements[index].Pos.y;
		vertex1.Pos.z	 = model->GetMesh()->elements[index].Pos.z;
		vertex1.UV.x	 = model->GetMesh()->elements[index].UV.x;
		vertex1.UV.y	 = model->GetMesh()->elements[index].UV.y;
		vertex1.Normal.x = model->GetMesh()->elements[index].Normal.x;
		vertex1.Normal.y = model->GetMesh()->elements[index].Normal.y;
		vertex1.Normal.z = model->GetMesh()->elements[index].Normal.z;
		index++;

		vertex2.Pos.x = model->GetMesh()->elements[index].Pos.x;
		vertex2.Pos.y = model->GetMesh()->elements[index].Pos.y;
		vertex2.Pos.z = model->GetMesh()->elements[index].Pos.z;
		vertex2.UV.x = model->GetMesh()->elements[index].UV.x;
		vertex2.UV.y = model->GetMesh()->elements[index].UV.y;
		vertex2.Normal.x = model->GetMesh()->elements[index].Normal.x;
		vertex2.Normal.y = model->GetMesh()->elements[index].Normal.y;
		vertex2.Normal.z = model->GetMesh()->elements[index].Normal.z;
		index++;

		vertex3.Pos.x = model->GetMesh()->elements[index].Pos.x;
		vertex3.Pos.y = model->GetMesh()->elements[index].Pos.y;
		vertex3.Pos.z = model->GetMesh()->elements[index].Pos.z;
		vertex3.UV.x = model->GetMesh()->elements[index].UV.x;
		vertex3.UV.y = model->GetMesh()->elements[index].UV.y;
		vertex3.Normal.x = model->GetMesh()->elements[index].Normal.x;
		vertex3.Normal.y = model->GetMesh()->elements[index].Normal.y;
		vertex3.Normal.z = model->GetMesh()->elements[index].Normal.z;
		index++;

		CalculatetangentBinormal2(vertex1, vertex2, vertex3, normal, tangent, binormal);
		
		model->GetMesh()->elements[index - 1].Normal.x = normal.x;
		model->GetMesh()->elements[index - 1].Normal.y = normal.y;
		model->GetMesh()->elements[index - 1].Normal.z = normal.z;
		model->GetMesh()->elements[index - 1].Tangent.x = tangent.x;
		model->GetMesh()->elements[index - 1].Tangent.y = tangent.y;
		model->GetMesh()->elements[index - 1].Tangent.z = tangent.z;
		model->GetMesh()->elements[index - 1].Bitangent.x = binormal.x;
		model->GetMesh()->elements[index - 1].Bitangent.y = binormal.y;
 		model->GetMesh()->elements[index - 1].Bitangent.z = binormal.z;

		model->GetMesh()->elements[index - 2].Normal.x = normal.x;
		model->GetMesh()->elements[index - 2].Normal.y = normal.y;
		model->GetMesh()->elements[index - 2].Normal.z = normal.z;
		model->GetMesh()->elements[index - 2].Tangent.x = tangent.x;
		model->GetMesh()->elements[index - 2].Tangent.y = tangent.y;
		model->GetMesh()->elements[index - 2].Tangent.z = tangent.z;
		model->GetMesh()->elements[index - 2].Bitangent.x = binormal.x;
		model->GetMesh()->elements[index - 2].Bitangent.y = binormal.y;
		model->GetMesh()->elements[index - 2].Bitangent.z = binormal.z;

		model->GetMesh()->elements[index - 3].Normal.x = normal.x;
		model->GetMesh()->elements[index - 3].Normal.y = normal.y;
		model->GetMesh()->elements[index - 3].Normal.z = normal.z;
		model->GetMesh()->elements[index - 3].Tangent.x = tangent.x;
		model->GetMesh()->elements[index - 3].Tangent.y = tangent.y;
		model->GetMesh()->elements[index - 3].Tangent.z = tangent.z;
		model->GetMesh()->elements[index - 3].Bitangent.x = binormal.x;
		model->GetMesh()->elements[index - 3].Bitangent.y = binormal.y;
		model->GetMesh()->elements[index - 3].Bitangent.z = binormal.z;

	}

}

void GameObject::CreateAndSetSamplerState()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

}

void GameObject::ModelToLoad(Model* model)
{
	this->model = model;
}

void GameObject::CreateAndSetTextureHeightWic(const wchar_t* filePath)
{
	HRESULT hr = CreateWICTextureFromFile(_pd3dDevice, filePath, nullptr, &_pTextureArray[2], 0);
}

void GameObject::CreateAndSetTextureWIC(const wchar_t* filePath)
{
	HRESULT hr = CreateWICTextureFromFile(_pd3dDevice, filePath, nullptr, &_pTextureRV, 0);
	
	if (FAILED(hr))
	{
		std::cout << "failed" << std::endl;
	}
	else
	{
		std::cout << "Success" << std::endl;
	}
}

void GameObject::CreateAndSetTextureArrayWIC(const wchar_t* filePath, const wchar_t* filePath2)
{
	HRESULT hr;

	hr = CreateWICTextureFromFile(_pd3dDevice, filePath, nullptr, &_pTextureArray[0], 0);

	if (FAILED(hr))
	{
		std::cout << "failed1" << std::endl;
	}
	else
	{
		std::cout << "Success1" << std::endl;
	}

	hr = CreateWICTextureFromFile(_pd3dDevice, filePath2, nullptr, &_pTextureArray[1], 0);

	if (FAILED(hr))
	{
		std::cout << "failed2" << std::endl;
	}
	else
	{
		std::cout << "Success2" << std::endl;
	}
}

void GameObject::CreateAndSetTextureDDS(const wchar_t* filePath)
{
	HRESULT hr = CreateDDSTextureFromFile(_pd3dDevice, filePath, nullptr, &_pTextureRV, 0);
}

void GameObject::SetScale(XMFLOAT3 scale)
{
	_scale = scale;
}

void GameObject::SetTranslation(XMFLOAT3 position)
{
	_position = position;
}

void GameObject::SetRotation(XMFLOAT3 rotation)
{
	_rotation = rotation;
}

void GameObject::Update(float t)
{
	XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	XMMATRIX rotation = XMMatrixRotationX(_rotation.x) * XMMatrixRotationY(_rotation.y) * XMMatrixRotationZ(_rotation.z);
	XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);

	XMStoreFloat4x4(&_world, scale * rotation * translation);
}

void GameObject::Draw(ID3D11Buffer* _pConstantBuffer, ID3D11DeviceContext* _pImmediateContext, ConstantBuffer& cb, XMMATRIX result)
{
	if (model == nullptr)
	{
		return;
	}
	XMMATRIX world = XMLoadFloat4x4(&_world);

	cb.World = XMMatrixTranspose(world);
	
	//material->Draw(_pImmediateContext, cb);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);	

	model->Draw(_pImmediateContext);
}

void GameObject::DrawDeferred(ID3D11Buffer* _pConstantBuffer, ID3D11DeviceContext* _pImmediateContext, DeferredConstantBuffer& cb, XMMATRIX result)
{
	if (model == nullptr)
	{
		return;
	}
	XMMATRIX world = XMLoadFloat4x4(&_world);

	cb.World = XMMatrixTranspose(world);

	//material->Draw(_pImmediateContext, cb);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	model->Draw(_pImmediateContext);
}
