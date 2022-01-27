//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
//d3d11.hはいらないかも
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
//xnamath.hはあんまりよくないのかも
#include "resource.h"

#define KEY_LEFT 1
#define KEY_RIGHT 2
#define KEY_UP 4
#define KEY_SPACE 8
#define KEY_E 16
#define KEY_P 32
#define KEY_1 64
#define KEY_2 128
#define KEY_B 256
#define KEY_R 512

enum class eScene {
    TITLE,      //0
    SELECT,     //1
    GAME,       //2
    CLEAR,      //3

    NONE        
};

static eScene mScene = eScene::TITLE;
static eScene mNextScene = eScene::NONE;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
};

struct CBNeverChanges
{
    XMMATRIX mView;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
    XMFLOAT4 vLightDir;
    XMFLOAT4 vLightColor;
    XMFLOAT4 vChangeColor;
};

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;    //この警告わかんね
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pCBNeverChanges = NULL;
ID3D11Buffer*                       g_pCBChangeOnResize = NULL;
ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
ID3D11ShaderResourceView*           g_pTextureRV = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;

ID3D11PixelShader* g_pPixelShaderGoal = NULL;

//２個目用
ID3D11VertexShader*                 g_pVertexShader2 = NULL;
ID3D11PixelShader*                  g_pPixelShader2 = NULL;
ID3D11Buffer*                       g_pVertexBuffer2 = NULL;
//ID3D11Buffer*                       g_pIndexBuffer2 = NULL;    //同じの使ってるからいらないわ
ID3D11ShaderResourceView*           g_pTextureRV2 = NULL;

//ひよこの頭用
ID3D11Buffer*                       g_pVertexBuffer3 = NULL;
//ID3D11Buffer*                     g_pIndexBuffer3 = NULL;      //同じの使ってるからいらないわ


ID3D11VertexShader*                 g_pVertexShader4 = NULL;
ID3D11PixelShader*                  g_pPixelShader4 = NULL;
ID3D11Buffer*                       g_pVertexBuffer4 = NULL;
ID3D11ShaderResourceView*           g_pTextureRV4 = NULL;

ID3D11ShaderResourceView*           g_pTextureRVClear = NULL;




XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor( 1.0f, 1.0f, 1.0f, 1.0f );
int                                 key_input = 0;
float                               time = 0.0f;



//--------------------------------------------------------------------------------------
// Forward declarations     プロトタイプ宣言
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();
void KeyInput();
int KeyInputTriggerSense();
int DrawStage();
int DrawDuck();
void PlayDuckAction(int* duck_action, int action_list_index);
void SceneManagement();
void ChangeTitleScene();
int RenderTitleScene();
int SetViewDir(float EyePosX, float EyePosY, float EyePosZ);
void JudgeGameClear();
void TlansitGameScene(float& ClearedTime);
void InitializeGameScene();
void ChangeClearScene();
int RenderClearScene();


//XMFLOAT3(幅、高さ、奥行),XMFLOAT2(テクスチャー)
// Create vertex buffer
SimpleVertex vertices[] =
{
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, //上面
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//下面
    { XMFLOAT3(1.0f, -0.5f, -1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, //左
    { XMFLOAT3(-1.0f, -0.5f, -1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(1.0f, -0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },  //右
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },//手前
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },  //奥
    { XMFLOAT3(1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
};

//ステージデータ
const int StageSize = 6;
const int StageLevel[36] = 
{
        2,2,2,1,1,1,
        2,1,1,2,0,0,
        2,1,1,2,0,0,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
};

const int StageGimmick[36] =
{
    0,0,0,0,0,1,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
};

//キャラクターの向きを持つデータをmod4で
static int CharacterDirection = 100000; //4の倍数じゃないとだめ,あひるのむきを計算
const int Mod = 4;

static int DuckActionMain[12] = {};         //アクションを保存する配列
static int DuckActionPattern1[8] = {};      //パターンアクションを保存する配列
static int mainCount = 0;                   //メインアクションに入れるためのカウント
static int pattern1Count = 0;               //パターンアクションに入れるためのカウント

static int duckX = 0;               //ひよこのX座標
static int duckY = StageSize - 1;   //ひよこのY座標
static int duckZ = 0;               //ひよこのZ座標

static bool GameClearFlag = false;


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine,_In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        KeyInput();
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640 * 2, 480 * 2 };   //ウィンドウサイズを大きくした
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"duck programming", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader 頂点シェーダーの作成-------------------------------------------------------------------------
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader2);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    //タイトル画面用
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader4);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout-----------------------------------------------------------------------------------------------------
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout---------------------------------------------------------------------------------------------------
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout-------------------------------------------------------------------------------------------------
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader-------------------------------------------------------------------------------------------------
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader　ピクセルシェーダー作る-------------------------------------------------------------------------------------------------
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader2);
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    // Compile the pixel shader-------------------------------------------------------------------------------------------------
    //ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"Tutorial07.fx", "PS2", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    //タイトル画面用
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader4);
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    //ゴールのシェーダー
    hr = CompileShaderFromFile(L"Tutorial07.fx", "PSGoal", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    //タイトル画面用
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderGoal);
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    //----------------------------------------------------------------------
    // 動かしたい本体の頂点データ
    // XMFLOAT3(幅、高さ、奥行),XMFLOAT3(法線ベクトル),XMFLOAT2(テクスチャー)
    // Create vertex buffer

    float RatioX = 1.2f;
    float RatioZ = 1.5f;
    float RatioY = 1.5f;

    SimpleVertex vertices2[] =
    {
        { XMFLOAT3(-1.0f /RatioX, 1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    };

    //-------------------------------------------------------------------
    //ステージの頂点バッファの設定
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    //動かしたい本体の頂点バッファ
    //D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData2;
    ZeroMemory(&InitData2, sizeof(InitData2));
    InitData2.pSysMem = vertices2;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData2, &g_pVertexBuffer2);
    if (FAILED(hr))
        return hr;

    RatioX = 2.0f;
    RatioY = 2.0f;
    RatioZ = 2.0f;

    SimpleVertex vertices3[] =
    {
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, -1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, -1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f / RatioX, 1.0f / RatioY, 1.0f / RatioZ),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    };


    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData3;
    ZeroMemory(&InitData3, sizeof(InitData3));
    InitData3.pSysMem = vertices3;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData3, &g_pVertexBuffer3);
    if (FAILED(hr))
        return hr;

    int Size = 13;

    SimpleVertex vertices4[] = {
        {XMFLOAT3(-1.0f * Size,-1.0f * Size, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)},
        {XMFLOAT3(1.0f * Size, -1.0f * Size, 0.0f),XMFLOAT3(0.0f, 0.0f,-1.0f),XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(1.0f * Size,  1.0f * Size, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(1.0f, 0.0f)},
        {XMFLOAT3(-1.0f * Size, 1.0f * Size, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f)},
    };

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData4;
    ZeroMemory(&InitData4, sizeof(InitData4));
    InitData4.pSysMem = vertices4;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData4, &g_pVertexBuffer4);
    if (FAILED(hr))
        return hr;

#if 0
    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );
#endif

    // Create index buffer
    // Create vertex buffer
    // 共通で使う頂点インデックス
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    //---------------------------------------------------------------------
    //　１つ目の、頂点インデックス設定
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

#if 0 //これ使ってないからいらないわ
    // ２つ目の、頂点インデックス設定
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer2);
    if (FAILED(hr))
        return hr;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer3);
    if (FAILED(hr))
        return hr;
#endif


    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    //----------------------------------------------------------------------
    // Load the Texture テクスチャの読み込み
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"stage.dds", NULL, NULL, &g_pTextureRV, NULL );
    if( FAILED( hr ) )
        return hr;

#if 1
    //２つ目のテクスチャの読み込み
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"hiyoko.dds", NULL, NULL, &g_pTextureRV2, NULL);
    if (FAILED(hr))
        return hr;

    //タイトルのテクスチャの読み込み
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"title.dds", NULL, NULL, &g_pTextureRV4, NULL);
    if (FAILED(hr))
        return hr;

    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"gameclear.dds", NULL, NULL, &g_pTextureRVClear, NULL);
    if (FAILED(hr))
        return hr;
#endif

    //--------------------------------------------------------------
    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix       //ここを変更すると視点変更できる
    XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    //g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );   //透視投影
    g_Projection = XMMatrixOrthographicLH(width / 50, (FLOAT)height / 50, 1.0f, 1000.0f);   //平行投影
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear ) g_pSamplerLinear->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pCBNeverChanges ) g_pCBNeverChanges->Release();
    if( g_pCBChangeOnResize ) g_pCBChangeOnResize->Release();
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();

    if (g_pPixelShaderGoal) g_pPixelShaderGoal->Release();

    //ひよこの描画に使うやつ
    if (g_pVertexBuffer2) g_pVertexBuffer2->Release();
    if (g_pVertexShader2) g_pVertexShader2->Release();
    if (g_pPixelShader2) g_pPixelShader2->Release();
    if (g_pTextureRV2) g_pTextureRV2->Release();

    if (g_pVertexBuffer3) g_pVertexBuffer3->Release();
   
    //タイトル画面用
    if (g_pVertexBuffer4) g_pVertexBuffer4->Release();
    if (g_pVertexShader4) g_pVertexShader4->Release();
    if (g_pPixelShader4) g_pPixelShader4->Release();
    if (g_pTextureRV4) g_pTextureRV4->Release();


}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render( )
{
    // Update our time

    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        time += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount64();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        time = ( dwTimeCur - dwTimeStart ) / 1000.0f;  //時間
    }
    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    //----------------------------------------------------------------------
    //
    // Clear the back buffer  背景のクリア
    //
    float ColorRGBList[3] = { 224, 255, 255 };      //0～255まで背景色の変更
    float ClearColor[4] = { ColorRGBList[0] / 255, ColorRGBList[1] / 255, ColorRGBList[2] / 255, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    SceneManagement();


    
    //XMVECTOR translate = XMVectorSet(1.0f, 2.0f, 3.0f, );

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}


/// <summary>
/// ステージの描画
/// </summary>
/// <returns>0</returns>
int DrawStage() {

    //g_World = XMMatrixTranslation(x, 0, 0);
    //g_World = XMMatrixRotationY( t ) * XMMatrixRotationX(t) * XMMatrixTranslation(t, 3, 3);   //Y回転処理(時間ごと回転)
    //g_World *= XMMatrixTranslationFromVector(translate);  //移動したかったけど、XMVECTORがよくわからん

#if 0
    // Modify the color
    g_vMeshColor.x = ( sinf( x * 10.0f ) + 1.0f ) * 0.5f;    //時間ごとに色の変化
    g_vMeshColor.y = ( cosf( x * 3.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.z = ( sinf( x * 5.0f ) + 1.0f ) * 0.5f;
#endif 
    //
    // Update variables that change once per frame
    //

        // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    CBChangesEveryFrame cb;
    //キューブのいろいろ

    g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(0, -1, (StageSize - 1) * 2 * sqrt(2)); //最初に描くステージ
    int cnt = 0;
    for (int Dep = 0; Dep < StageSize; Dep++) {
        for (int Wid = 0; Wid < StageSize; Wid++) {
            for (int Hig = 0; Hig < StageLevel[cnt]; Hig++) {
                int StageBoxPos[3] = { Wid - Dep, Hig -1, (StageSize - 1) * 2 - Wid - Dep}; //x,y,z

                g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(StageBoxPos[0] * sqrt(2), StageBoxPos[1], StageBoxPos[2] * sqrt(2));
                cb.mWorld = XMMatrixTranspose(g_World);
                cb.vMeshColor = g_vMeshColor;
                cb.vLightDir = XMFLOAT4(0.1f, 1.0f, -0.3f, 1.0f);
                cb.vLightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
                cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
                g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

                if (StageGimmick[StageSize * Dep + Wid] == 1) {
                    //
                    // Render the cube
                    //
                    if (GameClearFlag) {
                        cb.vChangeColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
                        g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
                    }
                    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
                    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
                    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                    g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);
                    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
                    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                    g_pImmediateContext->DrawIndexed(36, 0, 0);

                }
                else {
                    //
                    // Render the cube
                    //
                    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
                    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
                    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
                    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
                    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                    g_pImmediateContext->DrawIndexed(36, 0, 0);
                }

            }
            cnt++;
        }
    }

    return 0;
}


/// <summary>
/// ひよこを描画する
/// </summary>
/// <returns>0</returns>
int DrawDuck() {

    //------------------------------------------------------------------
//本体の描画
    UINT stride2 = sizeof(SimpleVertex);
    UINT offset2 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer2, &stride2, &offset2);   //頂点バッファをセットする

    float FirstPosition[3] = { 0 , -0.86f, (StageSize - 1) * 2};

#if 0
    //前に進むプログラム
    static bool KEYUP = true;
    static int Before = 0;
    if (key_input & KEY_UP) {
        if (KEYUP) {
            Before++;
            KEYUP = false;
        }
    }
    else KEYUP = true;
#endif


    //座標系ステージの一番奥を(0,0)にする右斜め下X正、左斜め下Y正

    int keyinputtrigger = KeyInputTriggerSense();
    static int duckActionMenu = KEY_1;
    if (keyinputtrigger & KEY_1) duckActionMenu = KEY_1;
    if (keyinputtrigger & KEY_2) duckActionMenu = KEY_2;
    if (keyinputtrigger & KEY_R) InitializeGameScene();

    switch (duckActionMenu) {
        case KEY_1:
            if (mainCount < 12) {   //12個まで入れられるように
                if (keyinputtrigger) {
                    //elseにしないとない部分の配列も参照しちゃう
                    if (keyinputtrigger & KEY_LEFT)         DuckActionMain[mainCount] = KEY_LEFT;
                    else if (keyinputtrigger & KEY_RIGHT)   DuckActionMain[mainCount] = KEY_RIGHT;
                    else if (keyinputtrigger & KEY_UP)      DuckActionMain[mainCount] = KEY_UP;
                    else if (keyinputtrigger & KEY_SPACE)   DuckActionMain[mainCount] = KEY_SPACE;
                    else if (keyinputtrigger & KEY_E)       DuckActionMain[mainCount] = KEY_E;
                    else if (keyinputtrigger & KEY_B) {             //一個前を取り消し
                        if (mainCount >= 1) {
                            DuckActionMain[mainCount - 1] = 0;
                            mainCount -= 2;
                        }
                    }
                    else mainCount--;
                    mainCount++;
                }
            }
            else {
                if (keyinputtrigger & KEY_B) {          //全部埋まってるときに取り消し
                    DuckActionMain[mainCount - 1] = 0;
                    mainCount -= 1;
                }
            }
            break;
        case KEY_2:
            if (pattern1Count < 8) {   //8個まで入れられるように
                if (keyinputtrigger) {
                    if (keyinputtrigger & KEY_LEFT) DuckActionPattern1[pattern1Count] = KEY_LEFT;
                    else if (keyinputtrigger & KEY_RIGHT) DuckActionPattern1[pattern1Count] = KEY_RIGHT;
                    else if (keyinputtrigger & KEY_UP) DuckActionPattern1[pattern1Count] = KEY_UP;
                    else if (keyinputtrigger & KEY_SPACE) DuckActionPattern1[pattern1Count] = KEY_SPACE;
                    else if (keyinputtrigger & KEY_B) {
                        if (pattern1Count >= 1) {
                            DuckActionPattern1[pattern1Count - 1] = 0;
                            pattern1Count -= 2;
                        }
                    }
                    else pattern1Count--;
                    pattern1Count++;
                }
            }
            else {
                if (keyinputtrigger & KEY_B) {
                    DuckActionPattern1[pattern1Count - 1] = 0;
                    pattern1Count -= 1;
                }
            }
            break;
    }

#if 0 //キー入力した瞬間に動かす
    if (keyinputtrigger & KEY_LEFT) {
        CharacterDirection--;   //左回転
    }
    if (keyinputtrigger & KEY_RIGHT) {
        CharacterDirection++;   //右回転
    }
    if (keyinputtrigger & KEY_UP) {
        if (duckZ == NextStageLevel) {                              //同じ高さだけ動ける
            //範囲外にでないようにする　範囲内にいるときだけ計算
            if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
            if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
        }
    }
    if (keyinputtrigger & KEY_SPACE) {
        if (NextStageLevel != 0) {      //ステージがない箇所に行かないようにする
            //範囲外にでないようにする
            if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
            if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
        }

        //高さが同じでもジャンプ出前に行けるようにしておく
    }
    duckZ = StageLevel[StageSize * duckY + duckX];  //高さの計算
#endif

    //再生システム
    static bool PlayFlag = false;   //再生フラグ
    static float beforeTime = 0;    //移動する一個前に時間
    const float FlameTime = 0.4f;   //一個の移動にかかる時間
    static int countPlay = 0;       //実行される行動の個数
    static int CountPattern1Play = 0;
    if (keyinputtrigger & KEY_P) PlayFlag = true;

    if (PlayFlag) {
#if 1
        if (countPlay < 12) {   //12個までしか保存してない
            if (beforeTime + FlameTime < time) {    //以前実行したものから
                if (DuckActionMain[countPlay] & KEY_E) {
                    PlayDuckAction(DuckActionPattern1, CountPattern1Play);
                    CountPattern1Play++;
                    if (CountPattern1Play >= 8) {           //配列の一番最後に0入れるとかでもいいかも？
                        countPlay++;
                        CountPattern1Play = 0;
                    }
                    else if (DuckActionPattern1[CountPattern1Play] == 0) {  //配列の中がなにもないとき
                        countPlay++;
                        CountPattern1Play = 0;
                    }
                }
                else {
                    PlayDuckAction(DuckActionMain,countPlay);
                    countPlay++;
                    if (countPlay >= 12) {}
                    else if (DuckActionMain[countPlay] == 0) {
                        PlayFlag = false;
                        countPlay = 0;
                        JudgeGameClear();
                    }
                }

                //duckZ = StageLevel[StageSize * duckY + duckX];  //高さの計算　ここはなくてもいい
                beforeTime = time;
            }
        }
        else {
            PlayFlag = false;
            countPlay = 0;
            JudgeGameClear();
        }
    }

    TlansitGameScene(beforeTime);

#endif

    duckZ = StageLevel[StageSize * duckY + duckX];  //高さの計算 



    float DuckPosition[3] = { FirstPosition[0] + duckX - duckY, FirstPosition[1] + duckZ, FirstPosition[2] - duckX - duckY };   //あひるの位置

    float DuckDir = (2 * (CharacterDirection % Mod) + 1) * XM_PI / 4;
    g_World = XMMatrixRotationY(DuckDir) * XMMatrixTranslation(DuckPosition[0] * sqrt(2), DuckPosition[1], DuckPosition[2] * sqrt(2));

    CBChangesEveryFrame cb;
    //CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    cb.vLightDir = XMFLOAT4(0.1f, 1.0f, -0.5f, 1.0f);
    cb.vLightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader(g_pVertexShader2, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShader(g_pPixelShader2, NULL, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV2);
    g_pImmediateContext->DrawIndexed(36, 0, 0);



    //あひるの頭の部分の描画------------------------------------------------------------
    UINT stride3 = sizeof(SimpleVertex);
    UINT offset3 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer3, &stride3, &offset3);   //頂点バッファをセットする

    g_World = XMMatrixRotationZ(XM_PI / 2) * XMMatrixTranslation(0.6f, 0,0) * XMMatrixRotationY(DuckDir) * XMMatrixTranslation(DuckPosition[0] * sqrt(2), DuckPosition[1] + 1.1666f, DuckPosition[2] * sqrt(2));

    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    cb.vLightDir = XMFLOAT4(0.1f, 1.0f, -0.5f, 1.0f);
    cb.vLightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader(g_pVertexShader2, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShader(g_pPixelShader2, NULL, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV2);
    g_pImmediateContext->DrawIndexed(36, 0, 0);

    return 0;
}

/// <summary>
/// キー入力をとる。
/// </summary>
void KeyInput() {
    key_input = 0;
    if (GetAsyncKeyState('A') & 0x8000)         key_input |= KEY_LEFT;
    if (GetAsyncKeyState('D') & 0x8000)         key_input |= KEY_RIGHT;
    if (GetAsyncKeyState('W') & 0x8000)         key_input |= KEY_UP;
    if (GetAsyncKeyState(' ') & 0x8000)         key_input |= KEY_SPACE;
    if (GetAsyncKeyState('P') & 0x8000)         key_input |= KEY_P;
    if (GetAsyncKeyState('1') & 0x8000)         key_input |= KEY_1;
    if (GetAsyncKeyState('2') & 0x8000)         key_input |= KEY_2;
    if (GetAsyncKeyState('E') & 0x8000)         key_input |= KEY_E;
    if (GetAsyncKeyState('B') & 0x8000)         key_input |= KEY_B;
    if (GetAsyncKeyState('R') & 0x8000)         key_input |= KEY_R;
}

/// <summary>
/// トリガセンスのキー入力を返す
/// </summary>
/// <returns>key_input_triggersense</returns>
int KeyInputTriggerSense() {
    static int beforeKeyInput2 = 0;
    int key_input_triggersense = 0;
    if (key_input & KEY_LEFT)   if (!(beforeKeyInput2 & KEY_LEFT))  key_input_triggersense |= KEY_LEFT; 
    if (key_input & KEY_RIGHT)  if (!(beforeKeyInput2 & KEY_RIGHT)) key_input_triggersense |= KEY_RIGHT; //右一回だけ
    if (key_input & KEY_UP)     if (!(beforeKeyInput2 & KEY_UP))    key_input_triggersense |= KEY_UP; 
    if (key_input & KEY_SPACE)  if (!(beforeKeyInput2 & KEY_SPACE)) key_input_triggersense |= KEY_SPACE;
    if (key_input & KEY_P)      if (!(beforeKeyInput2 & KEY_P))     key_input_triggersense |= KEY_P;
    if (key_input & KEY_1)      if (!(beforeKeyInput2 & KEY_1))     key_input_triggersense |= KEY_1;
    if (key_input & KEY_2)      if (!(beforeKeyInput2 & KEY_2))     key_input_triggersense |= KEY_2;
    if (key_input & KEY_E)      if (!(beforeKeyInput2 & KEY_E))     key_input_triggersense |= KEY_E;
    if (key_input & KEY_B)      if (!(beforeKeyInput2 & KEY_B))     key_input_triggersense |= KEY_B;
    if (key_input & KEY_R)      if (!(beforeKeyInput2 & KEY_R))     key_input_triggersense |= KEY_R;
    beforeKeyInput2 = key_input;
    return key_input_triggersense;
}

/// <summary>
/// ひよこのアクションを実行する。
/// </summary>
/// <param name="duck_action">アクションの配列</param>
/// <param name="action_list_index">実行したい配列のインデックス</param>
void PlayDuckAction(int *duck_action, int action_list_index) {
    //前後左右を計算しやすくするためのデータ
    const int dx[4] = { 1, 0, -1,  0 };
    const int dy[4] = { 0, 1,  0, -1 };

    int move = CharacterDirection % Mod;                            //動く方向を処理するための数値
    int NextX = duckX + dx[move];                                   //次のX座標
    int NextY = duckY + dy[move];                                   //次のY座標
    int NextStageLevel = StageLevel[StageSize * NextY + NextX];     //次のステージの高さ

    if (duck_action[action_list_index] & KEY_LEFT) {
        CharacterDirection--;   //左回転
    }
    if (duck_action[action_list_index] & KEY_RIGHT) {
        CharacterDirection++;   //右回転
    }
    if (duck_action[action_list_index] & KEY_UP) {
        if (duckZ == NextStageLevel) {                              //同じ高さだけ動ける
            //範囲外にでないようにする　範囲内にいるときだけ計算
            if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
            if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
        }
    }
    if (duck_action[action_list_index] & KEY_SPACE) {
        if (NextStageLevel != 0) {      //ステージがない箇所に行かないようにする
            //範囲外にでないようにする
            if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
            if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
        }

        //高さが同じでもジャンプで前進できるようにしておく
    }

}

/// <summary>
/// シーン管理を行う関数
/// </summary>
void SceneManagement() {
    if (mNextScene != eScene::NONE) {   //次のシーンがセットされていれば
        mScene = mNextScene;            //次のシーンをセットする
        mNextScene = eScene::NONE;      //次のシーン情報をクリア
    }

    switch (mScene) {
    case eScene::TITLE:
        ChangeTitleScene();
        RenderTitleScene();
        break;
    case eScene::SELECT:

        break;
    case eScene::GAME:

        SetViewDir(0.0f, 3.0f,-6.0f);
        DrawStage();    //ステージの描画
        DrawDuck();

        break;
    case eScene::CLEAR:

        ChangeClearScene();
        RenderClearScene();
        break;
    }
}

/// <summary>
/// spaceキーでタイトルからシーン遷移
/// </summary>
void ChangeTitleScene() {
    int InputKey = KeyInputTriggerSense();
    if (InputKey & KEY_SPACE) {
        mNextScene = eScene::GAME;
    }
}

/// <summary>
/// タイトルシーンの描画
/// </summary>
/// <returns>0</returns>
int RenderTitleScene() {
    SetViewDir(0.0f, 1.0f, -6.0f);

    // Set vertex buffer
    UINT stride4 = sizeof(SimpleVertex);
    UINT offset4 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer4, &stride4, &offset4);

    g_World = XMMatrixTranslation(0, 1, 0);

    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -1.0f, 1.0f);
    cb.vLightColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader(g_pVertexShader4, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShader(g_pPixelShader4, NULL, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV4);
    g_pImmediateContext->DrawIndexed(6, 0, 0);

;
    return 0;
}

/// <summary>
/// spaceキーでクリア画面からシーン遷移
/// </summary>
void ChangeClearScene() {
    int InputKey = KeyInputTriggerSense();
    if (InputKey & KEY_SPACE) {
        mNextScene = eScene::TITLE;
    }
}


/// <summary>
/// ゲームクリアシーンの描画
/// </summary>
/// <returns></returns>
int RenderClearScene() {
    SetViewDir(0.0f, 1.0f, -6.0f);

    // Set vertex buffer
    UINT stride4 = sizeof(SimpleVertex);
    UINT offset4 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer4, &stride4, &offset4);

    g_World = XMMatrixTranslation(0, 1, 0);

    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -1.0f, 1.0f);
    cb.vLightColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader(g_pVertexShader4, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShader(g_pPixelShader4, NULL, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVClear);
    g_pImmediateContext->DrawIndexed(6, 0, 0);

    ;
    return 0;
}


/// <summary>
/// カメラの向きを設定する
/// </summary>
/// <param name="EyePosX">目のX座標</param>
/// <param name="EyePosY">目のY座標</param>
/// <param name="EyePosZ">目のZ座標</param>
/// <returns>0</returns>
int SetViewDir(float EyePosX,float EyePosY,float EyePosZ) {

    // Initialize the view matrix       //ここを変更すると視点変更できる
    XMVECTOR Eye = XMVectorSet(EyePosX, EyePosY, EyePosZ, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose(g_View);
    g_pImmediateContext->UpdateSubresource(g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0);

    return 0;
}

/// <summary>
/// ゲームクリアの判定
/// </summary>
void JudgeGameClear() {
    if (StageGimmick[StageSize * duckY + duckX] == 1) {
        GameClearFlag = true;
    }
}

/// <summary>
/// ゲームシーンシーンから他のシーンへの遷移
/// </summary>
/// <param name="ClearedTime">クリアした瞬間の時間</param>
void TlansitGameScene(float& ClearedTime) {
    const float TransitBeforeTime = 1.0f;
    if (GameClearFlag) {
        if (ClearedTime + TransitBeforeTime < time) {
            mScene = eScene::CLEAR;
            InitializeGameScene();
        }
    }
}

/// <summary>
/// ゲームシーンの初期化を行う
/// </summary>
void InitializeGameScene() {
    GameClearFlag = false;
    CharacterDirection = 100000;
    duckX = 0;
    duckY = StageSize - 1;
    for (int i = 0; i < 12; i++)DuckActionMain[i] = 0;
    for (int i = 0; i < 8; i++) DuckActionPattern1[i] = 0;
    mainCount = 0;
    pattern1Count = 0;
}