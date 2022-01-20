//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"
#include <vector>

#define KEY_LEFT 1
#define KEY_RIGHT 2
#define KEY_UP 4
#define KEY_DOWN 8
#define KEY_SPACE 16
#define KEY_P 32

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
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
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
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

//２個目用
ID3D11VertexShader*                 g_pVertexShader2 = NULL;
ID3D11PixelShader*                  g_pPixelShader2 = NULL;
ID3D11Buffer*                       g_pVertexBuffer2 = NULL;
ID3D11Buffer*                       g_pIndexBuffer2 = NULL;
ID3D11ShaderResourceView*           g_pTextureRV2 = NULL;


XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor( 1.0f, 1.0f, 1.0f, 1.0f );
int                                 key_input = 0;


//--------------------------------------------------------------------------------------
// Forward declarations
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

//XMFLOAT3(幅、高さ、奥行),XMFLOAT2(テクスチャー)
// Create vertex buffer
SimpleVertex vertices[] =
{
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -0.5f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(1.0f, -0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
};

//ステージデータ
int StageSize = 4;
int StageLevel[16] = {
        2,2,2,1,
        2,1,1,0,
        2,1,2,0,
        1,1,1,1,
};

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
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

    // Create the vertex shader 頂点シェーダーの作成
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

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader　ピクセルシェーダー作る
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

    //----------------------------------------------------------------------
    // 動かしたい本体の頂点データ
    // XMFLOAT3(幅、高さ、奥行),XMFLOAT2(テクスチャー)
    // Create vertex buffer
    SimpleVertex vertices2[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
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

    // ２つ目の、頂点インデックス設定
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer2);
    if (FAILED(hr))
        return hr;

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
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"ahiru.dds", NULL, NULL, &g_pTextureRV2, NULL);
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

    // Initialize the view matrix
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

    if (g_pVertexBuffer2) g_pVertexBuffer2->Release();
    if (g_pVertexShader2) g_pVertexShader2->Release();
    if (g_pPixelShader2) g_pPixelShader2->Release();

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
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount64();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;  //時間
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

    DrawStage();    //ステージの描画

    DrawDuck();
    
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

    g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(0, -1, 3 * 2 * sqrt(2)); //最初に描くステージ
    int cnt = 0;
    for (int Wid = 0; Wid < StageSize; Wid++) {
        for (int dep = 0; dep < StageSize; dep++) {
            for (int Hig = 0; Hig < StageLevel[cnt]; Hig++) {
                cb.mWorld = XMMatrixTranspose(g_World);
                cb.vMeshColor = g_vMeshColor;
                g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

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
                g_World *= XMMatrixTranslation(0, 1, 0);
            }
            g_World *= XMMatrixTranslation(sqrt(2), -StageLevel[cnt], -sqrt(2));    //移動
            cnt++;
        }
        g_World *= XMMatrixTranslation((-StageSize - 1) * sqrt(2), 0, (StageSize - 1) * sqrt(2));
        //g_World *= XMMatrixTranslation(-(StageSize - 1 - (Wid + 1)) * sqrt(2), 0, -(StageSize - 1 + Wid + 1) * sqrt(2));
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

    float FirstPosition[3] = { 0 , -0.5f, (StageSize - 1) * 2};

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

#elif 0

    static int BeforeKeyInput = 0;

    if (key_input & KEY_DOWN) {
        if (key_input & KEY_DOWN & BeforeKeyInput) {

        }
        else Before--;
    }
    BeforeKeyInput = key_input;
#endif
    //座標系ステージの一番奥を(0,0)にする右斜め下X正、左斜め下Y正
    //プレイヤーをステージの上で動かす
    //キャラクターの向きを持つデータをmod4で
    static int CharacterDirection = 100000; //4の倍数じゃないとだめ,あひるのむきを計算
    int Mod = 4;

    //前後左右を計算しやすくするためのデータ
    const int dx[4] = { 1, 0, -1,  0};
    const int dy[4] = { 0, 1,  0, -1};

    static int duckX = 0;               //あひるのX座標
    static int duckY = StageSize - 1;   //あひるのY座標
    static int duckZ = 0;               //あひるのZ座標
    int keyinputtrigger = KeyInputTriggerSense();

#if 0
    static int action[12] = {};
    static int count = 0;
    if (keyinputtrigger) {
        if (keyinputtrigger & KEY_LEFT) action[count] = KEY_LEFT;
        if (keyinputtrigger & KEY_RIGHT) action[count] = KEY_RIGHT;
        if (keyinputtrigger & KEY_UP) action[count] = KEY_UP;
        if (keyinputtrigger & KEY_SPACE) action[count] = KEY_SPACE;
        count++;
    }
#endif

    int move = CharacterDirection % Mod;                            //動く方向を処理するための数値
    int NextX = duckX + dx[move];                                   //次のX座標
    int NextY = duckY + dy[move];                                   //次のY座標
    int NextStageLevel = StageLevel[StageSize * NextY + NextX];     //次のステージの高さ


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

#if 0
    if (keyinputtrigger & KEY_P) {
        for (int i = 0; i < 12; i++) {
            if (action[i] & KEY_LEFT) {
                CharacterDirection--;   //左回転
            }
            if (action[i] & KEY_RIGHT) {
                CharacterDirection++;   //右回転
            }
            if (action[i] & KEY_UP) {
                if (duckZ == NextStageLevel) {                              //同じ高さだけ動ける
                    //範囲外にでないようにする　範囲内にいるときだけ計算
                    if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
                    if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
                }
            }
            if (action[i] & KEY_SPACE) {
                if (NextStageLevel != 0) {      //ステージがない箇所に行かないようにする
                    //範囲外にでないようにする
                    if (!(NextX < 0 || StageSize - 1 < NextX))          duckX += dx[move];
                    if (!(NextY < 0 || StageSize - 1 < NextY))          duckY += dy[move];
                }

                //高さが同じでもジャンプ出前に行けるようにしておく
            }
            duckZ = StageLevel[StageSize * duckY + duckX];  //高さの計算
            
        }
    }
#endif

    

    float DuckPosition[3] = { FirstPosition[0] + duckX - duckY, FirstPosition[1] + duckZ, FirstPosition[2] - duckX - duckY };   //あひるの位置

    g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(DuckPosition[0] * sqrt(2), DuckPosition[1], DuckPosition[2] * sqrt(2));

    CBChangesEveryFrame cb;
    //CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
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
    if (GetAsyncKeyState('S') & 0x8000)         key_input |= KEY_DOWN;
    if (GetAsyncKeyState(' ') & 0x8000)         key_input |= KEY_SPACE;
    if (GetAsyncKeyState('P') & 0x8000)         key_input != KEY_P;
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
    if (key_input & KEY_DOWN)   if (!(beforeKeyInput2 & KEY_DOWN))  key_input_triggersense |= KEY_DOWN;
    if (key_input & KEY_SPACE)  if (!(beforeKeyInput2 & KEY_SPACE)) key_input_triggersense |= KEY_SPACE;
    if (key_input & KEY_P)      if (!(beforeKeyInput2 & KEY_P))     key_input_triggersense |= KEY_P;
    beforeKeyInput2 = key_input;
    return key_input_triggersense;
}

