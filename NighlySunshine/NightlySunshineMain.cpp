//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment (lib, "D3D10_1.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D2D1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <D3D10_1.h>
#include <DXGI.h>
#include <sstream>
#include <dwrite.h>
#include <dinput.h>
#include <d3dcompiler.h>
#include <ctime>

#include "sunpos.h"

#include "imgui.h"
#include "imgui_impl_dx11.hpp"
#include "LogWindow.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Mesh.hpp"
#include "Camera.hpp"

using namespace DirectX;

IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;

ID3D11VertexShader* VSShadow;
ID3D11PixelShader* PSShadow;
ID3D10Blob* VS_BufferShadow;
ID3D10Blob* PS_BufferShadow;

ID3D11InputLayout* vertLayout;
ID3D11Buffer* cbPerObjectBuffer;
ID3D11RasterizerState* CCWcullMode;
ID3D11RasterizerState* CWcullMode;

ID3D11Texture2D *BackBuffer11;

LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = nullptr;
HRESULT hr;

int Width = 1600;
int Height = 900;

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;
DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

XMMATRIX WVP;
D3D11_VIEWPORT viewport;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;

XMMATRIX groundWorld;

ID3D11Buffer *squareIndexBuffer;
ID3D11Buffer *squareVertBuffer;

Mesh m;

float rot = 0.01f;

double countsPerSecond = 0.0;
__int64 CounterStart = 0;

int frameCount = 0;
int fps = 0;

__int64 frameTimeOld = 0;
double frameTime;

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void DrawScene();
void UpdateScene(double time);

void StartTimer();
double GetTime();
double GetFrameTime();

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed);
extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int messageloop();

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct cbPerObject
{
	XMMATRIX ModelViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX  _World2Receiver;
	XMMATRIX _Object2World;
	XMMATRIX  _World2Object;
	XMVECTOR _Scale;
	XMVECTOR _WorldSpaceLightPos0;
	XMMATRIX  WVP;
	XMMATRIX World;
	XMVECTOR sunParam;
};

cbPerObject cbPerObj;

struct Vertex //Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), colour(r, g, b, a){}

	XMFLOAT3 pos;
	XMFLOAT4 colour;
};

Camera cam;
Camera shadowCam;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11App(hInstance))    //Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitScene())    //Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
		return 0;
	}
	ImGui_ImplDX11_Init(hwnd, d3d11Device, d3d11DevCon);
	
	messageloop();

	CleanUp();

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(NULL, WndClassName, L"Lesson 4 - Begin Drawing", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	//Describe our SwapChain Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGIFactory1 *DXGIFactory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&DXGIFactory); 
	IDXGIAdapter1 *Adapter;
	hr = DXGIFactory->EnumAdapters1(0, &Adapter);
	DXGIFactory->Release();

	//Create our Direct3D 11 Device and SwapChain//////////////////////////////////////////////////////////////////////////
	hr = D3D11CreateDeviceAndSwapChain(Adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, nullptr, &d3d11DevCon);

	//Release the Adapter interface
	Adapter->Release();

	//Create our BackBuffer and Render Target
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer11);
	hr = d3d11Device->CreateRenderTargetView(BackBuffer11, NULL, &renderTargetView);

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = Width;
	depthStencilDesc.Height = Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create the Depth/Stencil View
	d3d11Device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);

	return true;
}

bool InitDirectInput(HINSTANCE hInstance)
{
	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, nullptr);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, nullptr);
	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, nullptr);
	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

void DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	/*if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	}*/

	cam.DetectInput(keyboardState, mouseCurrState, mouseLastState, time);
	cam.UpdateCamera();

	return;
}

void CleanUp()
{
	SwapChain->SetFullscreenState(false, NULL);
	PostMessage(hwnd, WM_DESTROY, 0, 0);

	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
	cbPerObjectBuffer->Release();
	CCWcullMode->Release();
	CWcullMode->Release();
	BackBuffer11->Release();
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();
}

bool InitScene()
{
	cam.InitCamera(false, Width, Height);
	shadowCam.InitCamera(false, Width, Height);

	//Compile Shaders from shader file
	D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &VS_Buffer, nullptr);
	D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &PS_Buffer, nullptr);

	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

	D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "shadowmain", "vs_5_0", 0, 0, &VS_BufferShadow, nullptr);
	D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "shadowmain", "ps_5_0", 0, 0, &PS_BufferShadow, nullptr);
	hr = d3d11Device->CreateVertexShader(VS_BufferShadow->GetBufferPointer(), VS_BufferShadow->GetBufferSize(), NULL, &VSShadow);
	hr = d3d11Device->CreatePixelShader(PS_BufferShadow->GetBufferPointer(), PS_BufferShadow->GetBufferSize(), NULL, &PSShadow);

	m.LoadObj(d3d11Device, d3d11DevCon, "assets/testmesh.obj");

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);

	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	cmdesc.FrontCounterClockwise = true;
	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;

	hr = d3d11Device->CreateRasterizerState(&cmdesc, &CWcullMode);

	//Create the vertex buffer
	Vertex v[] =
	{
		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
	};

	DWORD indices[] = {
		0,  1,  2,
		0,  2,  3,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	
	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &squareVertBuffer);

	return true;
}

void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}

void UpdateScene(double time)
{
	//Reset cube1World
	groundWorld = XMMatrixIdentity();

	//Define cube1's world space matrix
	Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	//Set cube1's world space using the transformations
	groundWorld = Scale * Translation;
}

void DrawScene()
{
	bool show_another_window = false;
	ImVec4 clear_col = ImColor(114, 144, 154);
	static int date[3];
	static float time[3];
	static float latlong[2];
	static cSunCoordinates finalSunPos = cSunCoordinates();

	ImGui_ImplDX11_NewFrame();
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Outline", &show_another_window);
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::InputInt3("date (DD MM YYYY)", date);
			ImGui::InputFloat3("time (HH MM SS)", time);
			ImGui::InputFloat2("position (lat lon)", latlong);

			cTime datetime;
			datetime.iDay = date[0];
			datetime.iMonth = date[1];
			datetime.iYear = date[2];

			datetime.dHours = time[0];
			datetime.dMinutes = time[1];
			datetime.dSeconds = time[2];

			cLocation place;
			place.dLatitude = latlong[0];
			place.dLongitude = latlong[1];

			if (ImGui::Button("Calculate Sun position"))
			{
				sunpos(datetime, place, &finalSunPos);
				auto sun = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
				
				cbPerObj._WorldSpaceLightPos0 = XMVectorMultiply(XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), finalSunPos.dAzimuth), XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f));
				cbPerObj.sunParam = XMVectorSet(finalSunPos.dAzimuth, finalSunPos.dZenithAngle, 0.0f, 0.0f);
			}
			ImGui::Text("Sun Azimuth: %f | Sun Zenith Angle: %f", finalSunPos.dAzimuth, finalSunPos.dZenithAngle);
		}
		ImGui::End();
	}

	WVP = groundWorld * cam.camView * cam.camProjection;

	cbPerObj.ModelViewMatrix = XMMatrixIdentity() * cam.camView;
	cbPerObj.ProjectionMatrix = XMMatrixTranspose(cam.camProjection);
	cbPerObj.ViewMatrix = XMMatrixTranspose(cam.camView);
	cbPerObj._Object2World = XMMatrixTranspose(groundWorld);
	cbPerObj._World2Receiver = groundWorld;
	cbPerObj._World2Object = XMMatrixInverse(nullptr, XMMatrixIdentity());
	auto sun = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cbPerObj._WorldSpaceLightPos0 = XMLoadFloat4(&sun);
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.World = XMMatrixTranspose(XMMatrixIdentity());
	
	float bgColor[4] = { 0.9f, 0.9f, 0.1f, 1.0f };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	d3d11DevCon->IASetInputLayout(vertLayout);
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffffff);
	d3d11DevCon->RSSetViewports(1, &viewport);
	d3d11DevCon->RSSetState(CWcullMode);

	//draw the shadow
	d3d11DevCon->VSSetShader(VSShadow, 0, 0);
	d3d11DevCon->PSSetShader(PSShadow, 0, 0);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, nullptr, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	m.DrawMesh(d3d11DevCon);

	//draw the geometry
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, nullptr, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	m.DrawMesh(d3d11DevCon);

	ImGui::Render();
	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}

int messageloop() 
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run game code    
			frameCount++;
			if (GetTime() > 1.0f)
			{
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}

			frameTime = GetFrameTime();

			DetectInput(frameTime);
			UpdateScene(frameTime);
			DrawScene();
		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplDX11_WndProcHandler(hwnd, msg, wParam, lParam))
	{
		return true;
	}
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
	case WM_SIZE:
		if (d3d11Device != NULL)
		{
			ImGui_ImplDX11_InvalidateDeviceObjects();
			SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}