#include "DeferredRender.h"

ID3D11Texture2D* DeferredRender::_deferredRenderTexture[BUFFER_COUNT];
ID3D11RenderTargetView* DeferredRender::_deferredRenderTargetView[BUFFER_COUNT];
ID3D11ShaderResourceView* DeferredRender::_deferredShaderResourceView[BUFFER_COUNT];
ID3D11Buffer* DeferredRender::_vertexBuffer;
ID3D11Buffer* DeferredRender::_indexBuffer;
ID3D11VertexShader* DeferredRender::_vertexShader;
ID3D11PixelShader* DeferredRender::_pixelShader;
ID3D11VertexShader* DeferredRender::_lightingVertexShader;
ID3D11PixelShader* DeferredRender::_lightingPixelShader;

ID3D11InputLayout* DeferredRender::_vertexLayout;
ID3D11InputLayout* DeferredRender::_vertexLayoutLighting;

ID3D11Buffer* DeferredRender::_constantBufferGeometry = nullptr;
ID3D11Buffer* DeferredRender::_constantBufferLight = nullptr;

//Initalise Cb's
DeferredConstantBuffer DeferredRender::cbDeffered;
DeferredLightConstantBuffer DeferredRender::cbLight;

HRESULT DeferredRender::CompileShaderFromFile(const wchar_t* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

void DeferredRender::InitialisePostShader(ID3D11Device* _pd3dDevice, UINT _WindowHeight, UINT _WindowWidth)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	///////////////////////// Texture
   // Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc.Width = _WindowHeight;
	textureDesc.Height = _WindowWidth;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for(int i = 0; i < BUFFER_COUNT; i++)
	_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_deferredRenderTexture[i]);

	/////////////////////// Render Target
   // Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	for (int i = 0; i < BUFFER_COUNT; i++)
	_pd3dDevice->CreateRenderTargetView(_deferredRenderTexture[i], &renderTargetViewDesc, &_deferredRenderTargetView[i]);

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < BUFFER_COUNT; i++)
	_pd3dDevice->CreateShaderResourceView(_deferredRenderTexture[i], &shaderResourceViewDesc, &_deferredShaderResourceView[i]);

	SimpleVertex v[] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
	};

	WORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,
	};

	InitVertexBuffer(v, _pd3dDevice);
	InitIndexBuffer(indices, _pd3dDevice);

	InitialiseShaders(_pd3dDevice);


	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(DeferredConstantBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HRESULT hr = _pd3dDevice->CreateBuffer(&cbDesc, nullptr, &_constantBufferGeometry);

	if (FAILED(hr))
		std::cout << "Geometry constant buffer failed to create" << std::endl;
	else
		std::cout << "Geometry constant buffer succeded to create" << std::endl;

	D3D11_BUFFER_DESC cbLightDesc;
	ZeroMemory(&cbLightDesc, sizeof(cbLightDesc));
	cbLightDesc.Usage = D3D11_USAGE_DEFAULT;
	cbLightDesc.ByteWidth = sizeof(DeferredLightConstantBuffer);
	cbLightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbLightDesc.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&cbLightDesc, nullptr, &_constantBufferLight);

	if (FAILED(hr))
		std::cout << "Light constant buffer failed to create" << std::endl;
	else
		std::cout << "Light constant buffer succeded to create" << std::endl;
}

HRESULT DeferredRender::InitialiseShaders(ID3D11Device* _pd3dDevice)
{
	HRESULT hr;
	ID3DBlob* pVSBlob1 = nullptr;
	ID3DBlob* pVSBlob2 = nullptr;

	hr = CompileShaderFromFile(L"DeferredGeometry.fx", "VS", "vs_4_0", &pVSBlob1);
	if(FAILED(hr))
	{
		MessageBox(nullptr,
			L"The DeferredGeometry VS file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob1->GetBufferPointer(), pVSBlob1->GetBufferSize(), nullptr, &_vertexShader);

	if (FAILED(hr))
	{
		pVSBlob1->Release();
		return hr;
	}

	hr = CompileShaderFromFile(L"DeferredLighting.fx", "VS", "vs_4_0", &pVSBlob2);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The DeferredLighting VS file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &_lightingVertexShader);

	if (FAILED(hr))
	{
		pVSBlob2->Release();
		return hr;
	}

	ID3DBlob* pPSBlob1 = nullptr;
	ID3DBlob* pPSBlob2 = nullptr;
	hr = CompileShaderFromFile(L"DeferredGeometry.fx", "PS", "ps_4_0", &pPSBlob1);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The DeferredGeometry ps file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob1->GetBufferPointer(), pPSBlob1->GetBufferSize(), nullptr, &_pixelShader);
	pPSBlob1->Release();

	if (FAILED(hr))
		return hr;


	hr = CompileShaderFromFile(L"DeferredLighting.fx", "PS", "ps_4_0", &pPSBlob2);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The DeferredLighting ps file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &_lightingPixelShader);
	pPSBlob2->Release();

	if (FAILED(hr))
		return hr;


	D3D11_INPUT_ELEMENT_DESC layoutGeometry[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layoutGeometry);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutGeometry, numElements, pVSBlob1->GetBufferPointer(),
		pVSBlob1->GetBufferSize(), &_vertexLayout);
	pVSBlob1->Release();

	D3D11_INPUT_ELEMENT_DESC layoutLighting[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements2 = ARRAYSIZE(layoutLighting);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutLighting, numElements2, pVSBlob2->GetBufferPointer(),
		pVSBlob2->GetBufferSize(), &_vertexLayoutLighting);
	pVSBlob2->Release();


	return hr;
}

void DeferredRender::InitVertexBuffer(SimpleVertex v[], ID3D11Device* _pd3dDevice)
{
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = v;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_vertexBuffer);

	if (FAILED(hr))
		std::cout << "Failed to intialise vertex buffer" << std::endl;

}

void DeferredRender::InitIndexBuffer(WORD Indices[], ID3D11Device* _pd3dDevice)
{
	HRESULT hr;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = Indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_indexBuffer);

	if (FAILED(hr))
		std::cout << "Failed to intialise index buffer" << std::endl;
}

void DeferredRender::Render(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* _pImmediateContext, ID3D11DepthStencilView* _depthStencilView, GameObject* object, XMMATRIX view, XMMATRIX projection, Light light)
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha

	_pImmediateContext->OMSetRenderTargets(BUFFER_COUNT, _deferredRenderTargetView, _depthStencilView);
	for (size_t i = 0; i < BUFFER_COUNT; i++)
	{
		_pImmediateContext->ClearRenderTargetView(_deferredRenderTargetView[i], ClearColor);
	}

	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_pImmediateContext->IASetInputLayout(_vertexLayout);

	_pImmediateContext->VSSetShader(_vertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pixelShader, nullptr, 0);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_constantBufferGeometry);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_constantBufferGeometry);

	cbDeffered.View = XMMatrixTranspose(view);
	cbDeffered.Projection = XMMatrixTranspose(projection);

	if (object->HasTexture()) //With parallax Mapping
	{
		ID3D11ShaderResourceView** textureArray = object->GetTextureArray(); 
		_pImmediateContext->PSSetShaderResources(0, 3, textureArray);
	}

	object->DrawDeferred(_constantBufferGeometry, _pImmediateContext, cbDeffered, XMMatrixIdentity());


	_pImmediateContext->OMSetRenderTargets(1, &renderTargetView, _depthStencilView);
	_pImmediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_pImmediateContext->VSSetShader(_lightingVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_lightingPixelShader, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_constantBufferLight);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_constantBufferLight);

	cbLight.light = light;
	_pImmediateContext->UpdateSubresource(_constantBufferLight, 0, nullptr, &cbLight, 0, 0);


	UINT offset = 0;
	UINT stride = sizeof(SimpleVertex);

	_pImmediateContext->IASetInputLayout(_vertexLayoutLighting);
	_pImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	///*_pImmediateContext->PSSetShaderResources(0, 1, &resourceView);
	//_pImmediateContext->PSSetShaderResources(1, 1, &_dissolveHeightMap);
	//_pImmediateContext->PSSetShaderResources(2, 1, &_dissolveGradiant);
	//_pImmediateContext->PSSetShaderResources(3, 1, &_dissolveBeginTexture);*/

	_pImmediateContext->PSSetShaderResources(0, 1, &_deferredShaderResourceView[0]);
	_pImmediateContext->PSSetShaderResources(1, 1, &_deferredShaderResourceView[1]);

	_pImmediateContext->DrawIndexed(6, 0, 0);



	//_pImmediateContext->UpdateSubresource()
}