#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXMath.h>
#include "Camera.h"
#include "GameObject.h"
#include "resource.h"
#include "Struct.h"
#include "PostProcessing.h"
#include "DissolveShader.h"
#include "DeferredRender.h"

using namespace DirectX;

class Application
{
private:
	//RenderStates
	bool bool_Key_P;
	bool wirefreame_State;
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _noCull;

	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*			_pd3dDevice;
	ID3D11DeviceContext*	_pImmediateContext;
	IDXGISwapChain*			_pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*		_pVertexShader;
	ID3D11PixelShader*		_pPixelShader;
	ID3D11InputLayout*		_pVertexLayout;


	//Depth/Stencil Buffer
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;

	ID3D11Buffer* _pConstantBuffer;

	//Used for texturing
	ID3D11ShaderResourceView*	_pTextureRV = nullptr;
	ID3D11SamplerState*			_pSamplerLinear = nullptr;
	
	//Camera
	Camera* _camera;

	//lighting 
	DirectionalLight DL;
	MaterialStruct Mtrl;
	Light basicLight;


	//GameObject Items
	Model* _cubeModel;
	GameObject* _cubeObject;

	//Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void	Cleanup();
	HRESULT CompileShaderFromFile(const wchar_t* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	
	UINT _WindowHeight;
	UINT _WindowWidth;
public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update(float const deltaTime, float TimeRunning);
	void Draw();

};