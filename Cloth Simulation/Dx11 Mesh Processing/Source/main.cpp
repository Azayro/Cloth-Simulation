
//
// DirectX 11
//
// This demo shows how to use the Geometry shader (GS) and Stream Output (SO) to create a GPU-based particle system
//

#include <D3DX11.h>
#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <io.h>
#include <conio.h>
#include <vector>
#include "HLSLFactory.h"
#include "CGTextureLoader.h"
#include "CGClock.h"
#include "CGPivotCamera.h"
#include "CGPipeline.h"
#include "CGRasteriserStage.h"
#include "CGOutputMergerStage.h"
#include "buffers.h"
#include "CGModelInstance.h"
//#include "CGBasicTerrain.h"
#include "CGSnowParticles.h"
#include <CoreStructures\CoreStructures.h>
#include <CGModel\CGModel.h>
#include <Importers\CGImporters.h>

#include "Cloth.h"

using namespace std;


// Window definition variables

int								width = 600; // width of window
int								height = 600; // height of window
int								bitsPerPixel = 32; // colour depth
bool							windowed = true; // use window, not fullscreen (DirectX makes fullscreen easy to setup)

HWND							appWindow = nullptr; // main window handle
HDC								hDC	= nullptr; // windows device context


// DirectX setup variables

ID3D11Device					*device; // represent primary device (gpu)
ID3D11DeviceContext				*context; // immediate rendering context
D3D_FEATURE_LEVEL				supportedFeatureLevel;
DXGI_SWAP_CHAIN_DESC			sd;
IDXGISwapChain					*swapChain;

ID3D11Texture2D					*backBuffer = nullptr;
ID3D11Texture2D					*depthStencilBuffer = nullptr;
ID3D11RenderTargetView			*renderTargetView = nullptr;
ID3D11DepthStencilView			*depthStencilView = nullptr;


// Transform and lighting buffers (hosted in system memory)
cameraStruct					*cameraBuffer = nullptr;
worldTransformStruct			*worldTransformBuffer = nullptr;
gameTimeStruct					*gameTimeBuffer = nullptr;
lightModelStruct				*lightModelBuffer = nullptr;


// Transform and lighting cbuffers (represent corresponding T&L buffers in GPU memory)
ID3D11Buffer					*camera_cbuffer = nullptr;
ID3D11Buffer					*worldTransform_cbuffer = nullptr;
ID3D11Buffer					*gameTime_cbuffer = nullptr;
ID3D11Buffer					*lightModel_cbuffer = nullptr;


// Textures and texture resource views used to setup textures in C/C++ for use in HLSL
ID3D11ShaderResourceView		*clothSurfaceSRV = nullptr;
ID3D11SamplerState				*linearSampler = nullptr; // can be reused for any other texture, but setup here for use with the bee texture


// Main application variables

CGClock							*mainClock = nullptr;
CGPivotCamera					*cam = nullptr; // Main camera
vector<CGModelInstance*>		basicScene;
CGSnowParticleSystem			*snowSystem = nullptr;
CGPipeline						*basicTexturePipeline = nullptr; // Pipeline for snowy surface rendering

// Cloth
Cloth* cloth = nullptr;

//
// Declare function prototypes
//
int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int show_cmd);
LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void renderScene(void);


// ------------------------------

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int show_cmd) {

#pragma region Application bootstrap

	// Tell Windows to terminate app if heap becomes corrupted
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Initialise COM
	HRESULT hr = CoInitialize(NULL);

	if (!SUCCEEDED(hr))
		return -1;

	FILE *stdinFile = nullptr;
	FILE *stdoutFile = nullptr;
	FILE *stderrFile = nullptr;

	// Create a console for the application
	BOOL consoleSetup = AllocConsole();

	if (consoleSetup==TRUE) {

		// Re-direct stdin, stdout, stderr to the new console
		freopen_s(&stdinFile, "CONIN$", "r", stdin);
		freopen_s(&stdoutFile, "CONOUT$", "w", stdout);
		freopen_s(&stderrFile, "CONOUT$", "w", stderr);
		
		SetConsoleTitle(L"DirectX 11 debug console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	} else {
		
		// Console not created so re-direct stdout and stderr to debug files
		freopen_s(&stdoutFile, "runlog_stdout.txt", "w", stdout);
		freopen_s(&stderrFile, "runlog_stderr.txt", "w", stderr);
	}

	// Seed random number generator
	srand( (unsigned)time( NULL ) );
	rand();

#pragma endregion


#pragma region Application window setup

	// Initialise window class
	WNDCLASSEX								wndclass;

	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= WinProc; // event handler function
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= GetModuleHandle(NULL);
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_CROSS);
	wndclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= TEXT("DirectX11");
	wndclass.hIconSm		= NULL;

	// Register window class (must be done before we actually create an instance of the window)
	if (!RegisterClassEx(&wndclass)) {

		cout << "Cannot create application window class\n";
		return 0;
	}
	

	// Setup window rect and resize according to set styles
	RECT									windowRect;

	windowRect.left = (long)0;
	windowRect.right = (long)width;
	windowRect.top = (long)0;
	windowRect.bottom = (long)height;

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	// Adjust Window To True Requested Size
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	// Create main application window
	appWindow = CreateWindowEx(dwExStyle, TEXT("DirectX11"), TEXT("DirectX 11"), dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, 0, 0, windowRect.right-windowRect.left, windowRect.bottom-windowRect.top, NULL, NULL, h_instance, NULL);

	if (!appWindow) {

		cout << "Cannot create main application window\n";
		return 0;
	}

	if (windowed) {

		ShowWindow(appWindow, show_cmd);
		UpdateWindow(appWindow);
		SetFocus(appWindow);
	}

#pragma endregion


#pragma region DirectX setup - Create device, rendering context and swap chain

	// Setup swap chain description
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SampleDesc.Count = 1; // no multisampling
	sd.SampleDesc.Quality = 0;

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = appWindow;
	sd.Windowed     = windowed;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;


	// setup required feature levels
	D3D_FEATURE_LEVEL dxFeatureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0 // only interested in Dx11 support for this demo!
	};


	// notes: create swap chain with device (integrates creation in single step).  Luna does this seperately which requires interface level access to the relevant factory class interfaces (see Luna p. 109)
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SINGLETHREADED,
		dxFeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		&supportedFeatureLevel,
		&context);

#pragma endregion


#pragma region DirectX setup - Setup viewport for main window and setup default Rasteriser configuration

	// Setup a single viewport to cover the main application window
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	CGRasteriserStage *defaultRSStage = new CGRasteriserStage(device, 1, &viewport);

#pragma endregion


#pragma region DirectX setup - Setup default Output Merger pipeline stage

	//
	// Get the back buffer texture from swap chain and derive the associated (render target) view
	//
	
	// Get texture interface to back buffer (this is what we write into to display images on the screen)
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	// Create a render target view (RTV) so we can 'plug-in' the back buffer texture into the pipeline output
	hr = device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);

	//
	// Setup depth-stencil texture
	//

	D3D11_TEXTURE2D_DESC		depthStencilDesc;

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; // Multi-sample properties much match the above DXGI_SWAP_CHAIN_DESC (sd)
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
	hr = device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);

	//
	// Setup Output Merger stage
	//
	CGOutputMergerStage *defaultOMStage = new CGOutputMergerStage(device, 1, &renderTargetView, depthStencilView, nullptr, nullptr);

#pragma endregion


#pragma region Disabled Output Merger stage

	// By setting DepthEnable and StencilEnable to FALSE (and later setting the pixel shader to 0) we disable the rasterisation and subsequent fragment processing in a given pipeline. (See http://msdn.microsoft.com/en-us/library/windows/desktop/bb205125%28v=vs.85%29.aspx)

	D3D11_DEPTH_STENCIL_DESC dsDisableStateDesc;
	D3D11_BLEND_DESC blendDesc;

	ZeroMemory(&dsDisableStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	dsDisableStateDesc.DepthEnable = FALSE;
	dsDisableStateDesc.StencilEnable = FALSE;
	
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	CGOutputMergerStage *disabledOMStage = new CGOutputMergerStage(device, 1, &renderTargetView, depthStencilView, &dsDisableStateDesc, &blendDesc);

#pragma endregion


#pragma region Alpha-blending Output Merger stage

	D3D11_DEPTH_STENCIL_DESC dsStateDesc;
	D3D11_BLEND_DESC alphaBlendDesc;

	ZeroMemory(&dsStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ZeroMemory(&alphaBlendDesc, sizeof(D3D11_BLEND_DESC));

	dsStateDesc.DepthEnable = TRUE;
	dsStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsStateDesc.StencilEnable = FALSE;
	dsStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	alphaBlendDesc.AlphaToCoverageEnable = FALSE;
	alphaBlendDesc.IndependentBlendEnable = FALSE;
	alphaBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	alphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	CGOutputMergerStage *blendOMStage = new CGOutputMergerStage(device, 1, &renderTargetView, depthStencilView, &dsStateDesc, &alphaBlendDesc);

#pragma endregion


#pragma region Example texture setup

	// Texture example 1: Load snow surface DDS texture and associated resource view using D3DX
	hr = D3DX11CreateShaderResourceViewFromFile(device, L"Resources\\Textures\\superman.jpg", 0, 0, &clothSurfaceSRV, 0);


	//
	// Setup linear sampler object
	//
	D3D11_SAMPLER_DESC linearDesc;

	ZeroMemory(&linearDesc, sizeof(D3D11_SAMPLER_DESC));

	linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.MinLOD = 0.0f;
	linearDesc.MaxLOD = 0.0f;
	linearDesc.MipLODBias = 0.0f;
	linearDesc.MaxAnisotropy = 0; // unused for non-anisotropic filtering
	linearDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateSamplerState(&linearDesc, &linearSampler);

#pragma endregion

	
	// Create buffers in system memory to hold per-frame data to be passed to cbuffers
	cameraBuffer = (cameraStruct*)_aligned_malloc(sizeof(cameraStruct), 16);
	new (cameraBuffer)cameraStruct();

	worldTransformBuffer = (worldTransformStruct*)_aligned_malloc(sizeof(worldTransformStruct), 16);
	new (worldTransformBuffer)worldTransformStruct();

	gameTimeBuffer = (gameTimeStruct*)_aligned_malloc(sizeof(gameTimeStruct), 16);
	new (gameTimeBuffer)gameTimeStruct();	

	lightModelBuffer = (lightModelStruct*)_aligned_malloc(sizeof(lightModelStruct), 16);
	new (lightModelBuffer)lightModelStruct();


	// Setup cbuffer objects
	hr = createCBuffer(device, cameraBuffer, &camera_cbuffer);
	hr = createCBuffer(device, worldTransformBuffer, &worldTransform_cbuffer);
	hr = createCBuffer(device, gameTimeBuffer, &gameTime_cbuffer);
	hr = createCBuffer(device, lightModelBuffer, &lightModel_cbuffer);

	// Load shaders and setup pipeline models
	ID3DBlob *vsExtBytecode = nullptr;
	basicTexturePipeline = new CGPipeline(device, "Resources\\Shaders\\basic_tex_lighting_vs.hlsl", nullptr, "Resources\\Shaders\\basic_tex_lighting_ps.hlsl", nullptr, defaultRSStage, defaultOMStage, &vsExtBytecode);


	
	// Create main camera
	cam = new CGPivotCamera(-0.1f, 0.31f, 5.9f);

	// Setup models
	cloth = new Cloth(device, vsExtBytecode, 16, 16);
	// Setup scene objects
	basicScene.push_back(new CGModelInstance(cloth, XMFLOAT3(-0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)));


#pragma region Main event loop

	while(1) {

		MSG										msg;
	
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			if (WM_QUIT == msg.message)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		} else {

			// Update clock
			if (mainClock)
				mainClock->tick();
			else
				mainClock = new CGClock();
			
			// Display
			renderScene();
		}
	}

#pragma endregion


#pragma region Cleanup resources

	// Close main window
	BOOL teardownWindow = DestroyWindow(appWindow);

	// Flush remaining stream buffer content
	fflush(NULL);

	// Dispose of the console attached to the host process
	if (consoleSetup==TRUE) {

		cout << "\nPress any key to continue...";
		_getch();

		BOOL consoleTeardown = FreeConsole();
	}

	// Close file redirections
	if (stdinFile)
		fclose(stdinFile);

	if (stdoutFile)
		fclose(stdoutFile);
	
	if (stderrFile)
		fclose(stderrFile);


	// Shutdown COM
	CoUninitialize();

#pragma endregion

	return 0;
}


LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static BOOL			mDown = FALSE;
	static POINT		cPos, currentPos;

	switch(msg) {

#if 0
		case WM_ACTIVATEAPP:
			if (wparam==TRUE) // application is being activated
			else // application is being deactivated
			break;
#endif

		case WM_LBUTTONDOWN:

			SetCapture(hwnd);
			
			GetCursorPos(&cPos);
			SetCursorPos(256, 256);
			ShowCursor(FALSE);

			mDown = TRUE;

			break;


		case WM_MOUSEMOVE:
			
			if (mDown) {

				GetCursorPos(&currentPos);
				
				POINT disp;

				disp.x = currentPos.x - 256;
				disp.y = currentPos.y - 256;

				cam->rotateElevation((float)-disp.y * 0.01f);
				cam->rotateOnYAxis((float)-disp.x * 0.01f);

				SetCursorPos(256, 256);

				renderScene();
			}
			break;


		case WM_LBUTTONUP:

			if (mDown) {

				SetCursorPos(cPos.x, cPos.y);
				ShowCursor(TRUE);

				mDown = FALSE;
				ReleaseCapture();
			}
			break;


		case WM_MOUSEWHEEL:
			
			if ((short)HIWORD(wparam)<0)
				cam->zoomCamera(1.2f);
			if ((short)HIWORD(wparam)>0)
				cam->zoomCamera(0.9f);

			renderScene();
			break;


		case WM_KEYDOWN:

			switch(wparam) {

				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_SPACE:
					cloth->anchorOn = !cloth->anchorOn;
					break;

				default:
					return(DefWindowProc(hwnd, msg, wparam, lparam));
			}
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(hwnd, msg, wparam, lparam));
	}

	return 0;
}


void renderScene(void) 
{

	// Clear back buffer
	static const FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Get camera transformation matrices for the current frame
	XMMATRIX viewMatrix = cam->dxViewTransform();
	static const XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(3.142f * 0.5f, (float)width/(float)height, 0.1f, 500.0f);
	

	// Render scene objects

	// 1. Render terrain

	// setup cbuffers for the current frame
	cameraBuffer->viewProjMatrix = viewMatrix * projectionMatrix;
	XMVECTOR eyepos = cam->getCameraPos();
	XMStoreFloat3(&(cameraBuffer->eyePos), eyepos);

	gameTimeBuffer->gameTime = float(mainClock->gameTimeElapsed());
	
	lightModelBuffer->pointLight = CGPointLightStruct(
		XMFLOAT3(0.0f, 4.0f, 0.0f), // pos
		100.0f, // range
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), // ambient
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // diffuse
		XMFLOAT4(0.0f, 0.0f, 0.0f, 10.0f), // specular
		XMFLOAT3(0.0f, 1.0f, 0.0001f) // attenuation
	);

	mapBuffer<cameraStruct>(context, cameraBuffer, camera_cbuffer);
	mapBuffer<gameTimeStruct>(context, gameTimeBuffer, gameTime_cbuffer);
	mapBuffer<lightModelStruct>(context, lightModelBuffer, lightModel_cbuffer);


	// bind constant buffers to the relevant slots at each shader stage
	ID3D11Buffer* vsCBuffers[] = {camera_cbuffer, gameTime_cbuffer, worldTransform_cbuffer};
	ID3D11Buffer* psCBuffers[] = {camera_cbuffer, lightModel_cbuffer};

	context->VSSetConstantBuffers(0, 3, vsCBuffers);
	context->PSSetConstantBuffers(0, 2, psCBuffers);


	// Apply basic lighting pipeline
	basicTexturePipeline->applyPipeline(context);

	// Setup resources (could encapsulate texture in terrain mesh object if necessary)
	basicScene[0]->setupCBuffer(context, worldTransform_cbuffer);

	ID3D11ShaderResourceView* clothSRVs[] = {clothSurfaceSRV};
	context->PSSetShaderResources(0, 1, clothSRVs);
	context->PSSetSamplers(0, 1, &linearSampler);

	basicScene[0]->render(context);
	


	// Present current frame to the screen
	swapChain->Present(0, 0);
}
