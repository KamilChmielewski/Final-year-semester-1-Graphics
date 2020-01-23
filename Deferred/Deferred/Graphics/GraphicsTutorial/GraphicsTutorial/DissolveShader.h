//#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "Struct.h"
//
////struct SimpleVertex
////{
////	DirectX::XMFLOAT3 Pos;
////	DirectX::XMFLOAT3 Normal;
////	DirectX::XMFLOAT2 UV;
////};
//
class DissolveShader
{
private:
	static float time;
	static ID3D11Texture2D* _dissolveRenderTexture;

	static ID3D11ShaderResourceView* _dissolveShaderResourceView;
	static ID3D11ShaderResourceView* _dissolveHeightMap;
	static ID3D11ShaderResourceView* _dissolveGradiant;
	static ID3D11ShaderResourceView* _dissolveBeginTexture;


	static ID3D11Buffer* _vertexBuffer;
	static ID3D11Buffer* _indexBuffer;

	static ID3D11VertexShader* _vertexShader;
	static ID3D11PixelShader* _pixelShader;
	static ID3D11InputLayout* _vertexLayout;

	static DissolveConstantBuffer cb;
	static ID3D11Buffer* _pConstantBuffer2;

public:
	static ID3D11RenderTargetView* _dissolveRenderTargetView;

	static void	SetConstantBufferStartTime(float t);
	static void SetConstantBufferApplicationRunTime(float duration);
	static HRESULT CompileShaderFromFile(const wchar_t* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	static void InitialisePostShader(ID3D11Device* _pd3dDevice, UINT _WindowHeight, UINT _WindowWidth);
	static HRESULT InitialiseShaders(ID3D11Device* _pd3dDevice);
	static void InitVertexBuffer(SimpleVertex v[], ID3D11Device* _pd3dDevice);
	static void InitIndexBuffer(WORD Indices[], ID3D11Device* _pd3dDevice);
	static void Render(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* _pImmediateContext, ID3D11DepthStencilView* _depthStencilView, ID3D11ShaderResourceView* resourceView);
	static void LoadTextures(ID3D11Device* _pd3dDevice, const wchar_t* filePathHeight, const wchar_t* filePathGradiant, const wchar_t* filePathBeginTex);
};