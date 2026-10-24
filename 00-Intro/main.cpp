/* =============================================================
INTRODUCTION TO GAME PROGRAMMING SE102

SAMPLE 00 - INTRODUCTORY CODE

This sample illustrates how to:

1/ Create a window
2/ Initiate Direct3D 10, DirectX Sprite
3/ Draw a static brick sprite to the screen
4/ Create frame rate independent movements

5/ Some good C++ programming practices
- Use constants whenever possible
- 0 Warnings

6/ Debug using __FILE__ __LINE__

HOW TO INSTALL Microsoft.DXSDK.D3DX
===================================
1) Tools > NuGet package manager > Package Manager Console
2) execute command :  Install-Package Microsoft.DXSDK.D3DX


WARNING: This one file example has a hell LOT of *sinful* programming practices
================================================================ */

#include <windows.h>

#include <d3d10.h>
#include <d3dx10.h>

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

#include <comdef.h>
#include <math.h>

#define WINDOW_CLASS_NAME L"SampleWindow"
#define WINDOW_TITLE L"00 - Intro"
#define WINDOW_ICON_PATH L"brick.ico" 

HWND hWnd = 0;

// Each color is from 0.0f to 1.0f  ( 0/255 to 255/255 ) 
#define BACKGROUND_COLOR D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.2f)

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_FRAME_RATE 100

ID3D10Device* pD3DDevice = NULL;
IDXGISwapChain* pSwapChain = NULL;
ID3D10RenderTargetView* pRenderTargetView = NULL;

int BackBufferWidth = 0;
int BackBufferHeight = 0;

#define TEXTURE_PATH_BRICK L"brick.png"
#define BRICK_START_X 100.0f
#define BRICK_START_Y 50.0f

#define BRICK_START_VX 0.15f
#define BRICK_START_VY 0.15f

#define BRICK_WIDTH 16.0f
#define BRICK_HEIGHT 16.0f

#define PADDLE_WIDTH 80.0f
#define PADDLE_HEIGHT 16.0f
#define PADDLE_SPEED 0.35f
#define PADDLE_START_Y 420.0f


ID3D10Texture2D* texBrick = NULL;				// Texture object to store brick image
ID3DX10Sprite* spriteObject = NULL;				// Sprite handling object 

D3DX10_SPRITE spriteBrick;

float brick_x = BRICK_START_X;
float brick_vx = BRICK_START_VX;
float brick_y = BRICK_START_Y;
float brick_vy = BRICK_START_VY;

float paddle_x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2.0f;
float paddle_y = PADDLE_START_Y;
float paddle_vx = 0.0f;


LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// DEBUG SUPPORT FUNCTIONS //////////////
#define _W(x)  __W(x)
#define __W(x)  L##x

#define VA_PRINTS(s) {				\
		va_list argp;				\
		va_start(argp, fmt);		\
		vswprintf_s(s, fmt, argp);	\
		va_end(argp);				\
}		

void DebugOut(const wchar_t* fmt, ...)
{
	wchar_t s[4096];
	VA_PRINTS(s);
	OutputDebugString(s);
}

void DebugOutTitle(const wchar_t* fmt, ...)
{
	wchar_t s[1024];
	VA_PRINTS(s);
	SetWindowText(hWnd, s);
}
//////////////////////////////////////////

void InitDirectX(HWND hWnd)
{

	// retrieve client area width & height so that we can create backbuffer height & width accordingly 
	RECT r;
	GetClientRect(hWnd, &r);

	BackBufferWidth = r.right + 1;
	BackBufferHeight = r.bottom + 1;


	// Create & clear the DXGI_SWAP_CHAIN_DESC structure
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Fill in the needed values
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = BackBufferWidth;
	swapChainDesc.BufferDesc.Height = BackBufferHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	HRESULT hr = S_OK;

	//
	// Uncomment the following section to query graphic cards on the computer
	//  

	/*
	IDXGIFactory* pFactory = NULL;
	#pragma comment(lib, "dxgi")
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
	if (hr != S_OK)
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();

		DebugOut((wchar_t*)L"[ERROR] CreateDXGIFactory has failed %s %d %d %s\n", _W(__FILE__), __LINE__, hr, errMsg);
		return;
	}

	IDXGIAdapter* pAdapter = NULL;
	for (UINT i = 0; 
			pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; 
     ++i) 
	{
		DXGI_ADAPTER_DESC adapterDesc;
		pAdapter->GetDesc(&adapterDesc);
		DebugOut((wchar_t*)L"[INFO] Adapter %d: %s\n",i, adapterDesc.Description);
	}
	*/

	// Create the D3D device and the swap chain
	hr = D3D10CreateDeviceAndSwapChain(NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&swapChainDesc,
		&pSwapChain,
		&pD3DDevice);

	if (hr != S_OK)
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();

		DebugOut((wchar_t*)L"[ERROR] D3D10CreateDeviceAndSwapChain has failed %s %d %d %s\n", _W(__FILE__), __LINE__, hr, errMsg );
		return;
	}

	// Get the back buffer from the swapchain
	ID3D10Texture2D* pBackBuffer;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] pSwapChain->GetBuffer has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}

	// create the render target view
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);

	// release the back buffer
	pBackBuffer->Release();

	// Make sure the render target view was created successfully
	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] CreateRenderTargetView has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}

	// set the render target
	pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	// create and set the viewport
	D3D10_VIEWPORT viewPort;
	viewPort.Width = BackBufferWidth;
	viewPort.Height = BackBufferHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	pD3DDevice->RSSetViewports(1, &viewPort);


	// create the sprite object to handle sprite drawing 
	hr = D3DX10CreateSprite(pD3DDevice, 0, &spriteObject);

	// Make sure the sprite creation was successful
	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] D3DX10CreateSprite has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}

	D3DXMATRIX matProjection;

	// Create the projection matrix using the values in the viewport
	D3DXMatrixOrthoOffCenterLH(&matProjection,
		(float)viewPort.TopLeftX,
		(float)viewPort.Width,
		(float)viewPort.TopLeftY,
		(float)viewPort.Height,
		0.1f,
		10);
	hr = spriteObject->SetProjectionTransform(&matProjection);




	DebugOut((wchar_t*)L"[INFO] InitDirectX has been successful\n");

	return;
}

/*
	Load game resources. In this example, we only load a brick image
*/
void LoadResources()
{
	ID3D10Resource* pD3D10Resource = NULL;

	// Loads the texture into a temporary ID3D10Resource object
	HRESULT hr = D3DX10CreateTextureFromFile(pD3DDevice,
		TEXTURE_PATH_BRICK,
		NULL,
		NULL,
		&pD3D10Resource,
		NULL);

	// Make sure the texture was loaded successfully
	if (FAILED(hr))
	{
		DebugOut((wchar_t*)L"[ERROR] Failed to load texture file: %s \n", TEXTURE_PATH_BRICK);
		return;
	}

	// Translates the ID3D10Resource object into a ID3D10Texture2D object
	pD3D10Resource->QueryInterface(__uuidof(ID3D10Texture2D), (LPVOID*)&texBrick);
	pD3D10Resource->Release();

	if (!texBrick) {
		DebugOut((wchar_t*)L"[ERROR] Failed to convert from ID3D10Resource to ID3D10Texture2D \n");
		return;
	}

	// Get the texture details
	D3D10_TEXTURE2D_DESC desc;
	texBrick->GetDesc(&desc);

	// Create a shader resource view of the texture
	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;

	// Clear out the shader resource view description structure
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));

	// Set the texture format
	SRVDesc.Format = desc.Format;
	// Set the type of resource
	SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = desc.MipLevels;

	ID3D10ShaderResourceView* gSpriteTextureRV = NULL;

	pD3DDevice->CreateShaderResourceView(texBrick, &SRVDesc, &gSpriteTextureRV);

	// Set the sprite’s shader resource view
	spriteBrick.pTexture = gSpriteTextureRV;

	// top-left location in U,V coords
	spriteBrick.TexCoord.x = 0;
	spriteBrick.TexCoord.y = 0;

	// Determine the texture size in U,V coords
	spriteBrick.TexSize.x = 1.0f;
	spriteBrick.TexSize.y = 1.0f;

	// Set the texture index. Single textures will use 0
	spriteBrick.TextureIndex = 0;

	// The color to apply to this sprite, full color applies white.
	spriteBrick.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);


	DebugOut((wchar_t*)L"[INFO] Texture loaded Ok: %s \n", TEXTURE_PATH_BRICK);
}

/*
	Update world status for this frame
	dt: time period between beginning of last frame and beginning of this frame

	IMPORTANT: no render-related code should be used inside this function.
*/
void Update(DWORD dt)
{
	if (dt == 0) return;

	// 1. Paddle input handling (Left & Right arrow keys)
	paddle_vx = 0.0f;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		paddle_vx -= PADDLE_SPEED;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		paddle_vx += PADDLE_SPEED;
	}

	float prev_paddle_x = paddle_x;
	paddle_x += paddle_vx * dt;
	if (paddle_x < 0) paddle_x = 0;
	if (paddle_x > BackBufferWidth - PADDLE_WIDTH)
		paddle_x = (float)(BackBufferWidth - PADDLE_WIDTH);

	// 2. Continuous Collision Detection (CCD) for ball & paddle
	float prev_brick_x = brick_x;
	float prev_brick_y = brick_y;

	float next_brick_x = brick_x + brick_vx * dt;
	float next_brick_y = brick_y + brick_vy * dt;

	// Check top collision with paddle (when ball is moving down)
	bool hitPaddleTop = false;
	if (brick_vy > 0)
	{
		float prev_bottom = prev_brick_y + BRICK_HEIGHT;
		float next_bottom = next_brick_y + BRICK_HEIGHT;

		// Ball crossed the paddle top plane in this frame
		if (prev_bottom <= paddle_y && next_bottom >= paddle_y)
		{
			float t = 0.0f;
			if (next_bottom != prev_bottom)
				t = (paddle_y - prev_bottom) / (next_bottom - prev_bottom);

			if (t >= 0.0f && t <= 1.0f)
			{
				float hit_ball_x = prev_brick_x + brick_vx * dt * t;
				float hit_paddle_x = prev_paddle_x + paddle_vx * dt * t;

				// Check horizontal overlap at the exact moment of collision
				if (hit_ball_x + BRICK_WIDTH >= hit_paddle_x && hit_ball_x <= hit_paddle_x + PADDLE_WIDTH)
				{
					hitPaddleTop = true;

					// Place ball exactly on top of paddle surface
					brick_y = paddle_y - BRICK_HEIGHT;
					brick_x = hit_ball_x;

					// --- Realistic Physics Reflection ---
					// Preserve total kinetic energy / speed
					float current_speed = sqrtf(brick_vx * brick_vx + brick_vy * brick_vy);
					if (current_speed < 0.2f) current_speed = 0.2f;

					// Relative impact location [-1.0 .. 1.0] from center of paddle
					float paddleCenter = hit_paddle_x + PADDLE_WIDTH / 2.0f;
					float ballCenter = hit_ball_x + BRICK_WIDTH / 2.0f;
					float hitFactor = (ballCenter - paddleCenter) / (PADDLE_WIDTH / 2.0f);
					if (hitFactor < -1.0f) hitFactor = -1.0f;
					if (hitFactor > 1.0f) hitFactor = 1.0f;

					// Transfer horizontal momentum from moving paddle
					float paddleMomentum = (paddle_vx / PADDLE_SPEED) * 0.25f;
					hitFactor += paddleMomentum;
					if (hitFactor < -0.85f) hitFactor = -0.85f;
					if (hitFactor > 0.85f) hitFactor = 0.85f;

					// Bounce angle up to ~60 degrees from vertical
					const float MAX_BOUNCE_ANGLE = 3.14159265f / 3.0f; // 60 degrees
					float bounceAngle = hitFactor * MAX_BOUNCE_ANGLE;

					brick_vx = current_speed * sinf(bounceAngle);
					brick_vy = -current_speed * cosf(bounceAngle);

					// Advance remainder of frame time
					float remainingDt = (1.0f - t) * dt;
					brick_x += brick_vx * remainingDt;
					brick_y += brick_vy * remainingDt;
				}
			}
		}
	}

	if (!hitPaddleTop)
	{
		brick_x = next_brick_x;
		brick_y = next_brick_y;

		// Side collision handling: when ball hits the left or right edge of paddle
		if (brick_y + BRICK_HEIGHT > paddle_y && brick_y < paddle_y + PADDLE_HEIGHT)
		{
			// Left side of paddle
			if (brick_vx > 0 && brick_x + BRICK_WIDTH >= paddle_x && prev_brick_x + BRICK_WIDTH <= prev_paddle_x + 2.0f)
			{
				brick_x = paddle_x - BRICK_WIDTH;
				brick_vx = -fabsf(brick_vx);
			}
			// Right side of paddle
			else if (brick_vx < 0 && brick_x <= paddle_x + PADDLE_WIDTH && prev_brick_x >= prev_paddle_x + PADDLE_WIDTH - 2.0f)
			{
				brick_x = paddle_x + PADDLE_WIDTH;
				brick_vx = fabsf(brick_vx);
			}
		}
	}

	// 3. Wall collisions (Left, Right, Top)
	if (brick_x < 0)
	{
		brick_x = 0;
		brick_vx = fabsf(brick_vx);
	}
	else if (brick_x > BackBufferWidth - BRICK_WIDTH)
	{
		brick_x = (float)(BackBufferWidth - BRICK_WIDTH);
		brick_vx = -fabsf(brick_vx);
	}

	if (brick_y < 0)
	{
		brick_y = 0;
		brick_vy = fabsf(brick_vy);
	}

	// 4. Reset ball if it drops past bottom of screen
	if (brick_y > BackBufferHeight)
	{
		brick_x = BRICK_START_X;
		brick_y = BRICK_START_Y;
		brick_vx = BRICK_START_VX;
		brick_vy = BRICK_START_VY;
	}

	DebugOutTitle((wchar_t*)L"Ping Pong - Ball: (%0.1f, %0.1f) | Paddle: %0.1f", brick_x, brick_y, paddle_x);
}

/*
	Render a frame
	IMPORTANT: world status must NOT be changed during rendering
*/
void Render()
{
	if (pD3DDevice != NULL)
	{
		// clear the target buffer
		pD3DDevice->ClearRenderTargetView(pRenderTargetView, BACKGROUND_COLOR);

		// start drawing the sprites
		spriteObject->Begin(D3DX10_SPRITE_SORT_TEXTURE);

		// 1. Draw Ball (Brick)
		D3DXMATRIX matTranslation;
		D3DXMatrixTranslation(&matTranslation, brick_x, (BackBufferHeight - brick_y), 0.1f);
		D3DXMATRIX matScaling;
		D3DXMatrixScaling(&matScaling, BRICK_WIDTH, BRICK_HEIGHT, 1.0f);
		spriteBrick.matWorld = (matScaling * matTranslation);
		spriteObject->DrawSpritesImmediate(&spriteBrick, 1, 0, 0);

		// 2. Draw Paddle (Tiled 16x16 bricks for crisp pixel art visuals)
		for (float px = paddle_x; px < paddle_x + PADDLE_WIDTH - 0.1f; px += BRICK_WIDTH)
		{
			D3DXMATRIX matPaddleTranslation;
			D3DXMatrixTranslation(&matPaddleTranslation, px, (BackBufferHeight - paddle_y), 0.1f);
			D3DXMATRIX matPaddleScaling;
			D3DXMatrixScaling(&matPaddleScaling, BRICK_WIDTH, BRICK_HEIGHT, 1.0f);
			spriteBrick.matWorld = (matPaddleScaling * matPaddleTranslation);
			spriteObject->DrawSpritesImmediate(&spriteBrick, 1, 0, 0);
		}

		// Finish up and send the sprites to the hardware
		spriteObject->End();

		// display the next item in the swap chain
		pSwapChain->Present(0, 0);
	}
}

HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	//Try this to see how the debug function prints out file and line 
	//wc.hInstance = (HINSTANCE)-100; 

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(hInstance, WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd =
		CreateWindow(
			WINDOW_CLASS_NAME,
			WINDOW_TITLE,
			WS_OVERLAPPEDWINDOW, // WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenWidth,
			ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL);

	if (!hWnd)
	{
		DWORD ErrCode = GetLastError();
		DebugOut((wchar_t*)L"[ERROR] CreateWindow failed! ErrCode: %d\nAt: %s %d \n", ErrCode, _W(__FILE__), __LINE__);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG frameStart = GetTickCount64();
	ULONGLONG tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();

		// dt: the time between (beginning of last frame) and now
		// this frame: the frame we are about to render
		ULONGLONG dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;
			Update((DWORD)dt);
			Render();
		}
		else
			Sleep((DWORD)(tickPerFrame - dt));
	}

	return 1;
}

void Cleanup()
{
	// release the rendertarget
	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
	}
	// release the swapchain
	if (pSwapChain)
	{
		pSwapChain->Release();
	}
	// release the D3D Device
	if (pD3DDevice)
	{
		pD3DDevice->Release();
	}

	if (spriteObject)
	{
		spriteObject->Release();
		spriteObject = NULL;
	}

	DebugOut((wchar_t*)L"[INFO] Cleanup Ok\n");
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	hWnd = CreateGameWindow(hInstance, nCmdShow, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (hWnd == 0) return 0;

	InitDirectX(hWnd);

	LoadResources();
	Run();
	Cleanup();

	return 0;
}