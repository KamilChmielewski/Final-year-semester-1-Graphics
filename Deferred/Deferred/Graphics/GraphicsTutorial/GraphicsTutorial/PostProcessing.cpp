#include "PostProcessing.h"
#include <iostream>
#include <d3dcompiler.h>
 ID3D11Texture2D* PostProcessing::_postProcessRenderTexture;
 ID3D11RenderTargetView* PostProcessing::_postProcessRenderTargetView;
 ID3D11ShaderResourceView* PostProcessing::_postProcessShaderResourceView;
 ID3D11Buffer* PostProcessing::_vertexBuffer = nullptr;
 ID3D11Buffer* PostProcessing::_indexBuffer = nullptr;
 ID3D11InputLayout* PostProcessing::_vertexLayout = nullptr;
 ID3D11VertexShader* PostProcessing::_vertexShader = nullptr;
 ID3D11PixelShader* PostProcessing::_pixelShader = nullptr;

 HRESULT CompileShaderFromFile(const wchar_t* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

 ID3D11RenderTargetView* PostProcessing::GetRenderTargetView()
 {
	 return _postProcessRenderTargetView;
 }

 ID3D11ShaderResourceView* PostProcessing::GetShaderResourceView()
 {
	 return _postProcessShaderResourceView;
 }

 void PostProcessing::InitialisePostShader(ID3D11Device* _pd3dDevice, UINT _WindowHeight, UINT _WindowWidth)
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

	 _pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_postProcessRenderTexture);


	 /////////////////////// Render Target
	// Setup the description of the render target view.
	 renderTargetViewDesc.Format = textureDesc.Format;
	 renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	 renderTargetViewDesc.Texture2D.MipSlice = 0;

	 _pd3dDevice->CreateRenderTargetView(_postProcessRenderTexture, &renderTargetViewDesc, &_postProcessRenderTargetView);

	 shaderResourceViewDesc.Format = textureDesc.Format;
	 shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	 shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	 shaderResourceViewDesc.Texture2D.MipLevels = 1;

	 _pd3dDevice->CreateShaderResourceView(_postProcessRenderTexture, &shaderResourceViewDesc, &_postProcessShaderResourceView);

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
 }

 HRESULT PostProcessing::InitialiseShaders(ID3D11Device* _pd3dDevice)
 {
	 HRESULT hr;
	 ID3DBlob* pVSBlob = nullptr;

	 hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS2", "vs_4_0", &pVSBlob);

	 if (FAILED(hr))
	 {
		 MessageBox(nullptr,
			 L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		 return hr;
	 }

	 // Create the vertex shader
	 hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_vertexShader);

	 if (FAILED(hr))
	 {
		 pVSBlob->Release();
		 return hr;
	 }

	 ID3DBlob* pPSBlob = nullptr;
	 hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS2", "ps_4_0", &pPSBlob);

	 if (FAILED(hr))
	 {
		 MessageBox(nullptr,
			 L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		 return hr;
	 }

	 // Create the pixel shader
	 hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pixelShader);
	 pPSBlob->Release();

	 if (FAILED(hr))
		 return hr;

	 D3D11_INPUT_ELEMENT_DESC layout[] =
	 {
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		 { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		 { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 };

	 UINT numElements = ARRAYSIZE(layout);

	 // Create the input layout
	 hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		 pVSBlob->GetBufferSize(), &_vertexLayout);
	 pVSBlob->Release();

	 return hr;
 }


 void PostProcessing::InitVertexBuffer(SimpleVertex v[], ID3D11Device* _pd3dDevice)
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

 void PostProcessing::InitIndexBuffer(WORD Indices[], ID3D11Device* _pd3dDevice)
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
 

 void PostProcessing::Render(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* _pImmediateContext, ID3D11DepthStencilView* _depthStencilView)
 {
	 float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	 _pImmediateContext->OMSetRenderTargets(1, &renderTargetView, _depthStencilView);
	 _pImmediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
	 _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	 _pImmediateContext->VSSetShader(_vertexShader, nullptr, 0);
	 _pImmediateContext->PSSetShader(_pixelShader,  nullptr, 0);

	 UINT offset = 0;
	 UINT stride = sizeof(SimpleVertex);

	 _pImmediateContext->IASetInputLayout(_vertexLayout);
	 _pImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	 _pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	 _pImmediateContext->PSSetShaderResources(1, 1, &_postProcessShaderResourceView);

	 _pImmediateContext->DrawIndexed(6, 0, 0);
 }