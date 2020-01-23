#pragma once
#include "GameObject.h"
#include "Struct.h"

const int BUFFER_COUNT = 2;

class DeferredRender
{
private:
	static ID3D11Texture2D* _deferredRenderTexture[BUFFER_COUNT];
	static ID3D11ShaderResourceView* _deferredShaderResourceView[BUFFER_COUNT];

	static ID3D11Buffer* _vertexBuffer;
	static ID3D11Buffer* _indexBuffer;

	static ID3D11VertexShader* _vertexShader;
	static ID3D11PixelShader* _pixelShader;
	static ID3D11VertexShader* _lightingVertexShader;
	static ID3D11PixelShader* _lightingPixelShader;

	static ID3D11InputLayout* _vertexLayout;
	static ID3D11InputLayout* _vertexLayoutLighting;

	static ID3D11Buffer* _constantBufferGeometry;
	static ID3D11Buffer* _constantBufferLight;

	static DeferredConstantBuffer cbDeffered;
	static DeferredLightConstantBuffer cbLight;

public:
	static ID3D11RenderTargetView* _deferredRenderTargetView[BUFFER_COUNT];
	//static ID3D11RenderTargetView* GetRenderTargetView();
	//static ID3D11ShaderResourceView* GetShaderResourceView();

	static HRESULT CompileShaderFromFile(const wchar_t* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	static void InitialisePostShader(ID3D11Device* _pd3dDevice, UINT _WindowHeight, UINT _WindowWidth);
	static HRESULT InitialiseShaders(ID3D11Device* _pd3dDevice);
	static void InitVertexBuffer(SimpleVertex v[], ID3D11Device* _pd3dDevice);
	static void InitIndexBuffer(WORD Indices[], ID3D11Device* _pd3dDevice);
	static void Render(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* _pImmediateContext, ID3D11DepthStencilView* _depthStencilView, GameObject* object, XMMATRIX view, XMMATRIX projection, Light light);

};