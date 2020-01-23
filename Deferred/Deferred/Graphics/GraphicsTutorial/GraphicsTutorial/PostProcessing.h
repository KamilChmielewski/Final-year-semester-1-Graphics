#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "Struct.h"

//struct SimpleVertex
//{
//	DirectX::XMFLOAT3 Pos;
//	DirectX::XMFLOAT3 Normal;
//	DirectX::XMFLOAT2 UV;
//};

class PostProcessing
{
private:
	static ID3D11Texture2D* _postProcessRenderTexture;
	static ID3D11ShaderResourceView* _postProcessShaderResourceView;

	static ID3D11Buffer* _vertexBuffer;
	static ID3D11Buffer* _indexBuffer;

	static ID3D11VertexShader* _vertexShader;
	static ID3D11PixelShader* _pixelShader;
	static ID3D11InputLayout* _vertexLayout;

public:
	static ID3D11RenderTargetView* _postProcessRenderTargetView;
	static ID3D11RenderTargetView* GetRenderTargetView();
	static ID3D11ShaderResourceView* GetShaderResourceView();

	static void InitialisePostShader(ID3D11Device* _pd3dDevice, UINT _WindowHeight, UINT _WindowWidth);
	static HRESULT InitialiseShaders(ID3D11Device* _pd3dDevice);
	static void InitVertexBuffer(SimpleVertex v[], ID3D11Device* _pd3dDevice);
	static void InitIndexBuffer(WORD Indices[], ID3D11Device* _pd3dDevice);
	static void Render(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* _pImmediateContext, ID3D11DepthStencilView* _depthStencilView);
};