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
#define KEY_3 1024
#define KEY_ESC 2048
#define KEY_UPARROW 4096
#define KEY_LEFTARROW 8192
#define KEY_RIGHTARROW 16384

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
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;    //���̌x���킩���
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

//�Q�ڗp
ID3D11VertexShader*                 g_pVertexShader2 = NULL;
ID3D11PixelShader*                  g_pPixelShader2 = NULL;
ID3D11Buffer*                       g_pVertexBuffer2 = NULL;
//ID3D11Buffer*                       g_pIndexBuffer2 = NULL;    //�����̎g���Ă邩�炢��Ȃ���
ID3D11ShaderResourceView*           g_pTextureRV2 = NULL;

//�Ђ悱�̓��p
ID3D11Buffer*                       g_pVertexBuffer3 = NULL;
//ID3D11Buffer*                     g_pIndexBuffer3 = NULL;      //�����̎g���Ă邩�炢��Ȃ���


ID3D11VertexShader*                 g_pVertexShader4 = NULL;
ID3D11PixelShader*                  g_pPixelShader4 = NULL;
ID3D11Buffer*                       g_pVertexBuffer4 = NULL;
ID3D11ShaderResourceView*           g_pTextureRVTitle = NULL;

ID3D11ShaderResourceView*           g_pTextureRVClear = NULL;

ID3D11Buffer*                       g_pVertexBufferGameUI = NULL;

ID3D11ShaderResourceView*           g_pTextureRVForward = NULL;
ID3D11ShaderResourceView*           g_pTextureRVRight = NULL;
ID3D11ShaderResourceView*           g_pTextureRVLeft = NULL;
ID3D11ShaderResourceView*           g_pTextureRVJump = NULL;
ID3D11ShaderResourceView*           g_pTextureRVPattern = NULL;

ID3D11ShaderResourceView*           g_pTextureRVSelect = NULL;

ID3D11Buffer*                       g_pVertexBufferSelectUI = NULL;



XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor( 1.0f, 1.0f, 1.0f, 1.0f );
int                                 key_input = 0;
float                               time = 0.0f;



//--------------------------------------------------------------------------------------
// Forward declarations     �v���g�^�C�v�錾
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();
void KeyInput();
int KeyInputTriggerSense();
void DrawStage();
void DrawDuck();
void PlayDuckAction(int* duck_action, int action_list_index);
void SceneManagement();
void TransitTitleScene();
void RenderTitleScene();
void SetViewDir(float EyePosX, float EyePosY, float EyePosZ);
void JudgeGameClear();
void TlansitGameScene(float& ClearedTime);
void InitializeGameScene();
void TransitClearScene();
void RenderClearScene();
void RenderGameUI();
void RenderSelectScene();
void KeyInputSelectScene();
void SelectStage();

//XMFLOAT3(���A�����A���s),XMFLOAT2(�e�N�X�`���[)
// Create vertex buffer
SimpleVertex vertices[] =
{
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, //���
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//����
    { XMFLOAT3(1.0f, -0.5f, -1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, //��
    { XMFLOAT3(-1.0f, -0.5f, -1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(1.0f, -0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },  //�E
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },//��O
    { XMFLOAT3(1.0f, -0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, -1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },  //��
    { XMFLOAT3(1.0f, -0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.5f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.5f, 1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
};

//�X�e�[�W�f�[�^
int StageSize = 6;
int StageLevel[36] = 
{
        2,2,2,1,1,2,
        2,1,1,2,0,0,
        2,1,1,2,0,0,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
};

//�X�e�[�W�̃S�[���A�M�~�b�N�̈ʒu�f�[�^
int StageGimmick[36] =
{
    0,0,0,0,0,1,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
    0,0,0,0,0,0,
};

int StageLevelDatas[3][36]{
    {
        2,2,2,1,1,2,
        2,1,1,2,0,0,
        2,1,1,2,0,0,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
        1,1,1,1,1,1,
    },
    {
        3,3,3,3,3,3,
        3,3,3,5,4,3,
        3,3,3,3,3,2,
        3,3,3,3,3,1,
        3,3,3,3,3,1,
        1,1,1,1,1,1,
    },
    {
        2,2,2,2,2,2,
        2,2,2,2,2,2,
        2,2,2,2,2,2,
        2,2,2,2,2,2,
        2,2,2,2,2,1,
        1,1,1,1,1,1,
    }
};

int StageGimmickDatas[3][36]{
    {
        0,0,0,0,0,1,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
    },
    {
        0,0,0,0,0,0,
        0,0,0,1,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,
    },
    {
        0,0,0,0,0,1,
        0,0,0,0,0,2,
        2,0,0,0,0,0,
        0,0,0,0,0,2,
        2,0,0,0,0,0,
        0,0,0,0,0,2,
    },

};

//�L�����N�^�[�̌��������f�[�^��mod4��
static int CharacterDirection = 100000; //4�̔{������Ȃ��Ƃ���,���Ђ�̂ނ����v�Z
const int Mod = 4;

static int DuckActionMain[12] = {};         //�A�N�V������ۑ�����z��
static int DuckActionPattern1[8] = {};      //�p�^�[���A�N�V������ۑ�����z��
static int mainCount = 0;                   //���C���A�N�V�����ɓ���邽�߂̃J�E���g
static int pattern1Count = 0;               //�p�^�[���A�N�V�����ɓ���邽�߂̃J�E���g
    
static bool PlayOnceFlag = true;    //��񂾂��Đ��ł���悤��

static int duckX = 0;               //�Ђ悱��X���W
static int duckY = StageSize - 1;   //�Ђ悱��Y���W
static int duckZ = 0;               //�Ђ悱��Z���W

static bool GameClearFlag = false;

static int DuckActionMenu = KEY_1;  //�A�N�V�������j���[�̑I��

static int SelectNum = 0;           //�X�e�[�W�Z���N�g�̔ԍ�

static int JudgeStandMap[36] = {};



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
    RECT rc = { 0, 0, 640 * 2, 480 * 2 };   //�E�B���h�E�T�C�Y��傫������
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

    // Create the vertex shader ���_�V�F�[�_�[�̍쐬-------------------------------------------------------------------------
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

    //�^�C�g����ʗp
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

    // Create the pixel shader�@�s�N�Z���V�F�[�_�[���-------------------------------------------------------------------------------------------------
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

    //�^�C�g����ʗp
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader4);
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    //�S�[���̃V�F�[�_�[
    hr = CompileShaderFromFile(L"Tutorial07.fx", "PSGoal", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    //�^�C�g����ʗp
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderGoal);
    if (FAILED(hr)) {
        pPSBlob->Release();
        return hr;
    }

    //----------------------------------------------------------------------
    // �����������{�̂̒��_�f�[�^
    // XMFLOAT3(���A�����A���s),XMFLOAT3(�@���x�N�g��),XMFLOAT2(�e�N�X�`���[)
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
    //�X�e�[�W�̒��_�o�b�t�@�̐ݒ�
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

    //�����������{�̂̒��_�o�b�t�@
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
    InitData3.pSysMem = vertices3;                  //�|�C���^�[��n���Ă�̂ŁA���f�[�^���K�v�Avertices2�͎g���Ȃ�
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData3, &g_pVertexBuffer3);
    if (FAILED(hr))
        return hr;

    float Size = 13;

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


    //UI�̐����`�Ɏg��
    Size = 1.0f;

    SimpleVertex vertices5[] = {
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
    D3D11_SUBRESOURCE_DATA InitData5;
    ZeroMemory(&InitData5, sizeof(InitData5));
    InitData5.pSysMem = vertices5;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData5, &g_pVertexBufferGameUI);
    if (FAILED(hr))
        return hr;

    Size = 1.5f;

    SimpleVertex vertices6[] = {
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
    D3D11_SUBRESOURCE_DATA InitData6;
    ZeroMemory(&InitData6, sizeof(InitData6));
    InitData6.pSysMem = vertices6;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData6, &g_pVertexBufferSelectUI);
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
    // ���ʂŎg�����_�C���f�b�N�X
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
    //�@�P�ڂ́A���_�C���f�b�N�X�ݒ�
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
    // Load the Texture �e�N�X�`���̓ǂݍ���
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"stage.dds", NULL, NULL, &g_pTextureRV, NULL );
    if( FAILED( hr ) )
        return hr;

#if 1
    //�Q�ڂ̃e�N�X�`���̓ǂݍ���
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"hiyoko.dds", NULL, NULL, &g_pTextureRV2, NULL);
    if (FAILED(hr))
        return hr;

    //�^�C�g���̃e�N�X�`���̓ǂݍ���
    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"title.dds", NULL, NULL, &g_pTextureRVTitle, NULL);
    if (FAILED(hr))
        return hr;

    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"gameclear.dds", NULL, NULL, &g_pTextureRVClear, NULL);
    if (FAILED(hr))
        return hr;

    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"forward.dds", NULL, NULL, &g_pTextureRVForward, NULL);
    if (FAILED(hr))
        return hr;


    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"left.dds", NULL, NULL, &g_pTextureRVLeft, NULL);
    if (FAILED(hr))
        return hr;


    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"right.dds", NULL, NULL, &g_pTextureRVRight, NULL);
    if (FAILED(hr))
        return hr;


    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"jump.dds", NULL, NULL, &g_pTextureRVJump, NULL);
    if (FAILED(hr))
        return hr;

    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pattern.dds", NULL, NULL, &g_pTextureRVPattern, NULL);
    if (FAILED(hr))
        return hr;

    hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"select.dds", NULL, NULL, &g_pTextureRVSelect, NULL);
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

    // Initialize the view matrix       //������ύX����Ǝ��_�ύX�ł���
    XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    //g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );   //�������e
    g_Projection = XMMatrixOrthographicLH(width / 50, (FLOAT)height / 50, 1.0f, 1000.0f);   //���s���e
    
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

    //�Ђ悱�̕`��Ɏg�����
    if (g_pVertexBuffer2) g_pVertexBuffer2->Release();
    if (g_pVertexShader2) g_pVertexShader2->Release();
    if (g_pPixelShader2) g_pPixelShader2->Release();
    if (g_pTextureRV2) g_pTextureRV2->Release();

    if (g_pVertexBuffer3) g_pVertexBuffer3->Release();
   
    //�^�C�g����ʗp
    if (g_pVertexBuffer4) g_pVertexBuffer4->Release();
    if (g_pVertexShader4) g_pVertexShader4->Release();
    if (g_pPixelShader4) g_pPixelShader4->Release();
    if (g_pTextureRVTitle) g_pTextureRVTitle->Release();

    if (g_pTextureRVClear) g_pTextureRVClear->Release();

    if (g_pVertexBufferGameUI) g_pVertexBufferGameUI->Release();

    if (g_pTextureRVForward) g_pTextureRVForward->Release();
    if (g_pTextureRVRight) g_pTextureRVRight->Release();
    if (g_pTextureRVLeft) g_pTextureRVLeft->Release();
    if (g_pTextureRVJump) g_pTextureRVJump->Release();
    if (g_pTextureRVPattern) g_pTextureRVPattern->Release();
    if (g_pTextureRVSelect) g_pTextureRVSelect->Release();

    if (g_pVertexBufferSelectUI) g_pVertexBufferSelectUI->Release();

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
        time = ( dwTimeCur - dwTimeStart ) / 1000.0f;  //����
    }
    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    //----------------------------------------------------------------------
    //
    // Clear the back buffer  �w�i�̃N���A
    //
    float ColorRGBList[3] = { 224, 255, 255 };      //0�`255�܂Ŕw�i�F�̕ύX
    float ClearColor[4] = { ColorRGBList[0] / 255, ColorRGBList[1] / 255, ColorRGBList[2] / 255, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    SceneManagement();


    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}

/// <summary>
/// �V�[���Ǘ����s���֐�
/// </summary>
void SceneManagement() {
    if (mNextScene != eScene::NONE) {   //���̃V�[�����Z�b�g����Ă����
        mScene = mNextScene;            //���̃V�[�����Z�b�g����
        mNextScene = eScene::NONE;      //���̃V�[�������N���A
    }

    switch (mScene) {
    case eScene::TITLE:
        TransitTitleScene();
        RenderTitleScene();
        break;
    case eScene::SELECT:
        KeyInputSelectScene();
        RenderSelectScene();
        SelectStage();
        break;
    case eScene::GAME:
        SetViewDir(0.0f, 3.0f, -6.0f);

        DrawStage();    //�X�e�[�W�̕`��
        DrawDuck();
        RenderGameUI();

        break;
    case eScene::CLEAR:

        TransitClearScene();
        RenderClearScene();
        break;
    }
}

/// <summary>
/// �X�e�[�W�̕`��
/// </summary>
void DrawStage() {
    //
    // Update variables that change once per frame
    //

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    CBChangesEveryFrame cb;

    //�L���[�u�̕`��

    g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(0, -1, (StageSize - 1) * 2 * sqrt(2)); //�ŏ��ɕ`���X�e�[�W�̈ʒu

    for (int Dep = 0; Dep < StageSize; Dep++) {
        for (int Wid = 0; Wid < StageSize; Wid++) {
            int IndexStageData = StageSize * Dep + Wid;
            for (int Hig = 0; Hig < StageLevel[IndexStageData]; Hig++) {
                int StageBoxPos[3] = { Wid - Dep, Hig -1, (StageSize - 1) * 2 - Wid - Dep}; //x,y,z�@�`�悷��X�e�[�W�̈ʒu
                //DirectX�̍��W�n�ɕϊ�
                g_World = XMMatrixRotationY(XM_PI / 4) * XMMatrixTranslation(StageBoxPos[0] * sqrt(2), StageBoxPos[1], StageBoxPos[2] * sqrt(2));

                cb.mWorld = XMMatrixTranspose(g_World);
                cb.vMeshColor = g_vMeshColor;
                cb.vLightDir = XMFLOAT4(0.1f, 1.0f, -0.3f, 1.0f);
                cb.vLightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
                cb.vChangeColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
                g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

                if (StageGimmick[IndexStageData] == 1) {    //�S�[���̕`��
                    if (StageLevel[IndexStageData] - 1 == Hig) {
                        if (GameClearFlag) {
                            cb.vChangeColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
                            g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
                        }
                        g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);      //�V�F�[�_�[�ύX
                    }

                }
                else if (StageGimmick[IndexStageData] == 2) {
                    if (StageLevel[IndexStageData] - 1 == Hig) {
                        if (JudgeStandMap[IndexStageData] == 1) {
                            cb.vChangeColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
                        }
                        else {
                            cb.vChangeColor = XMFLOAT4(0.0f, 0.4f, 1.0f, 1.0f);
                        }
                        g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
                        g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);
                    }
                }
                else {
                    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

                }
                //
                // Render the cube
                //
                g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
                g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
                g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
                g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
                g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
                g_pImmediateContext->DrawIndexed(36, 0, 0);
            }
        }
    }
}


/// <summary>
/// �Ђ悱��`�悷��
/// </summary>
void DrawDuck() {

    //------------------------------------------------------------------
//�{�̂̕`��
    UINT stride2 = sizeof(SimpleVertex);
    UINT offset2 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer2, &stride2, &offset2);   //���_�o�b�t�@���Z�b�g����

    float FirstPosition[3] = { 0 , -0.86f, (StageSize - 1) * 2};

    //���W�n�X�e�[�W�̈�ԉ���(0,0)�ɂ���E�΂߉�X���A���΂߉�Y��

    int keyinputtrigger = KeyInputTriggerSense();
    
    if (keyinputtrigger & KEY_1) DuckActionMenu = KEY_1;
    if (keyinputtrigger & KEY_2) DuckActionMenu = KEY_2;
    if (keyinputtrigger & KEY_R) InitializeGameScene();

    switch (DuckActionMenu) {
        case KEY_1:
            if (mainCount < 12) {   //12�܂œ������悤��
                if (keyinputtrigger) {
                    //else�ɂ��Ȃ��ƂȂ������̔z����Q�Ƃ����Ⴄ
                    if (keyinputtrigger & KEY_LEFT)         DuckActionMain[mainCount] = KEY_LEFT;
                    else if (keyinputtrigger & KEY_RIGHT)   DuckActionMain[mainCount] = KEY_RIGHT;
                    else if (keyinputtrigger & KEY_UP)      DuckActionMain[mainCount] = KEY_UP;
                    else if (keyinputtrigger & KEY_SPACE)   DuckActionMain[mainCount] = KEY_SPACE;
                    else if (keyinputtrigger & KEY_E)       DuckActionMain[mainCount] = KEY_E;
                    else if (keyinputtrigger & KEY_B) {             //��O��������
                        if (mainCount >= 1) {
                            DuckActionMain[mainCount - 1] = 0;
                            mainCount--;
                        }
                        mainCount--;
                    }
                    else mainCount--;
                    mainCount++;
                }
            }
            else {
                if (keyinputtrigger & KEY_B) {          //�S�����܂��Ă�Ƃ��Ɏ�����
                    DuckActionMain[mainCount - 1] = 0;
                    mainCount -= 1;
                }
            }
            break;
        case KEY_2:
            if (pattern1Count < 8) {   //8�܂œ������悤��
                if (keyinputtrigger) {
                    if (keyinputtrigger & KEY_LEFT) DuckActionPattern1[pattern1Count] = KEY_LEFT;
                    else if (keyinputtrigger & KEY_RIGHT) DuckActionPattern1[pattern1Count] = KEY_RIGHT;
                    else if (keyinputtrigger & KEY_UP) DuckActionPattern1[pattern1Count] = KEY_UP;
                    else if (keyinputtrigger & KEY_SPACE) DuckActionPattern1[pattern1Count] = KEY_SPACE;
                    else if (keyinputtrigger & KEY_B) {
                        if (pattern1Count >= 1) {
                            DuckActionPattern1[pattern1Count - 1] = 0;
                            pattern1Count--;
                        }
                        pattern1Count--;
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

    //�Đ��V�X�e��
    static bool PlayFlag = false;   //�Đ��t���O
    static float beforeTime = 0;    //�ړ������O�Ɏ���
    const float FlameTime = 0.4f;   //��̈ړ��ɂ����鎞��
    static int countPlay = 0;       //���s�����s���̌�
    static int CountPattern1Play = 0;

    if (keyinputtrigger & KEY_P) {
        if (PlayOnceFlag) {
            PlayFlag = true;
            PlayOnceFlag = false;
        }
    }

    if (PlayFlag) {
#if 1
        if (countPlay < 12) {   //12�܂ł����ۑ����ĂȂ�
            if (beforeTime + FlameTime < time) {    //�ȑO���s�������̂���
                if (DuckActionMain[countPlay] & KEY_E) {
                    PlayDuckAction(DuckActionPattern1, CountPattern1Play);
                    CountPattern1Play++;
                    if (CountPattern1Play >= 8) {           //�z��̈�ԍŌ��0�����Ƃ��ł����������H
                        countPlay++;
                        CountPattern1Play = 0;
                    }
                    else if (DuckActionPattern1[CountPattern1Play] == 0) {  //�z��̒����Ȃɂ��Ȃ��Ƃ�
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

                //duckZ = StageLevel[StageSize * duckY + duckX];  //�����̌v�Z�@�����͂Ȃ��Ă�����
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
    if (keyinputtrigger & KEY_ESC) {
        mNextScene = eScene::SELECT;
        InitializeGameScene();
    }

#endif

    duckZ = StageLevel[StageSize * duckY + duckX];  //�����̌v�Z 

    JudgeStandMap[StageSize * duckY + duckX] = 1;   //���̏ꏊ��ʂ����̂��ǂ����̔���

    float DuckPosition[3] = { FirstPosition[0] + duckX - duckY, FirstPosition[1] + duckZ, FirstPosition[2] - duckX - duckY };   //���Ђ�̈ʒu

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



    //���Ђ�̓��̕����̕`��------------------------------------------------------------
    UINT stride3 = sizeof(SimpleVertex);
    UINT offset3 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer3, &stride3, &offset3);   //���_�o�b�t�@���Z�b�g����

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
}

/// <summary>
/// �L�[���͂��Ƃ�B
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
    if (GetAsyncKeyState('3') & 0x8000)         key_input |= KEY_3;
    if (GetAsyncKeyState('E') & 0x8000)         key_input |= KEY_E;
    if (GetAsyncKeyState('B') & 0x8000)         key_input |= KEY_B;
    if (GetAsyncKeyState('R') & 0x8000)         key_input |= KEY_R;
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)   key_input |= KEY_ESC;
    if (GetAsyncKeyState(VK_UP) & 0x8000)       key_input |= KEY_UPARROW;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)       key_input |= KEY_LEFTARROW;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)       key_input |= KEY_RIGHTARROW;
}

/// <summary>
/// �g���K�Z���X�̃L�[���͂�Ԃ�
/// </summary>
/// <returns>key_input_triggersense</returns>
int KeyInputTriggerSense() {
    static int beforeKeyInput2 = 0;
    int key_input_triggersense = 0;
    if (key_input & KEY_LEFT)   if (!(beforeKeyInput2 & KEY_LEFT))  key_input_triggersense |= KEY_LEFT; 
    if (key_input & KEY_RIGHT)  if (!(beforeKeyInput2 & KEY_RIGHT)) key_input_triggersense |= KEY_RIGHT; //�E��񂾂�
    if (key_input & KEY_UP)     if (!(beforeKeyInput2 & KEY_UP))    key_input_triggersense |= KEY_UP; 
    if (key_input & KEY_SPACE)  if (!(beforeKeyInput2 & KEY_SPACE)) key_input_triggersense |= KEY_SPACE;
    if (key_input & KEY_P)      if (!(beforeKeyInput2 & KEY_P))     key_input_triggersense |= KEY_P;
    if (key_input & KEY_1)      if (!(beforeKeyInput2 & KEY_1))     key_input_triggersense |= KEY_1;
    if (key_input & KEY_2)      if (!(beforeKeyInput2 & KEY_2))     key_input_triggersense |= KEY_2;
    if (key_input & KEY_3)      if (!(beforeKeyInput2 & KEY_3))     key_input_triggersense |= KEY_3;
    if (key_input & KEY_E)      if (!(beforeKeyInput2 & KEY_E))     key_input_triggersense |= KEY_E;
    if (key_input & KEY_B)      if (!(beforeKeyInput2 & KEY_B))     key_input_triggersense |= KEY_B;
    if (key_input & KEY_R)      if (!(beforeKeyInput2 & KEY_R))     key_input_triggersense |= KEY_R;
    if (key_input & KEY_ESC)    if (!(beforeKeyInput2 & KEY_ESC))   key_input_triggersense |= KEY_ESC;
    if (key_input & KEY_UPARROW)if (!(beforeKeyInput2 & KEY_UPARROW))key_input_triggersense |= KEY_UPARROW;
    if (key_input & KEY_LEFTARROW)    if (!(beforeKeyInput2 & KEY_LEFTARROW))   key_input_triggersense |= KEY_LEFTARROW;
    if (key_input & KEY_RIGHTARROW)    if (!(beforeKeyInput2 & KEY_RIGHTARROW))   key_input_triggersense |= KEY_RIGHTARROW;
    beforeKeyInput2 = key_input;
    return key_input_triggersense;
}

/// <summary>
/// �Ђ悱�̃A�N�V���������s�A�Đ�����B
/// </summary>
/// <param name="duck_action">�A�N�V�����̔z��</param>
/// <param name="action_list_index">���s�������z��̃C���f�b�N�X</param>
void PlayDuckAction(int *duck_action, int action_list_index) {
    //�O�㍶�E���v�Z���₷�����邽�߂̃f�[�^
    const int dx[4] = { 1, 0, -1,  0 };
    const int dy[4] = { 0, 1,  0, -1 };

    int move = CharacterDirection % Mod;                            //�����������������邽�߂̐��l
    int NextX = duckX + dx[move];                                   //����X���W
    int NextY = duckY + dy[move];                                   //����Y���W
    int NextStageLevel = 0;                                         //���̃X�e�[�W�̍���
    //�X�e�[�W�O�ɂ������Ȃ��悤��
    if (NextX < 0 || StageSize - 1 < NextX) {
        NextStageLevel = 0;
    }
    else if (NextY < 0 || StageSize - 1 < NextY) {
        NextStageLevel = 0;
    }
    else {
        NextStageLevel = StageLevel[StageSize * NextY + NextX];
    }


    if (duck_action[action_list_index] & KEY_LEFT) {
        CharacterDirection--;   //����]
    }
    if (duck_action[action_list_index] & KEY_RIGHT) {
        CharacterDirection++;   //�E��]
    }
    if (duck_action[action_list_index] & KEY_UP) {
        if (duckZ == NextStageLevel) {   //������������������
            duckX += dx[move];
            duckY += dy[move];
        }
    }
    if (duck_action[action_list_index] & KEY_SPACE) {
        if (NextStageLevel > 0) {      //�X�e�[�W���Ȃ��ӏ��ɍs���Ȃ��悤�ɂ���
            //�����������ł��W�����v�őO�i�ł���悤�ɂ��Ă���
            duckX += dx[move];
            duckY += dy[move];
        }
    }

    duckZ = StageLevel[StageSize * duckY + duckX];  //�����̌v�Z 
}

/// <summary>
/// space�L�[�Ń^�C�g������V�[���J��
/// </summary>
void TransitTitleScene() {
    int InputKey = KeyInputTriggerSense();
    if (InputKey & KEY_SPACE) {
        mNextScene = eScene::SELECT;
    }
}

/// <summary>
/// �^�C�g���V�[���̕`��
/// </summary>
void RenderTitleScene() {
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
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVTitle);
    g_pImmediateContext->DrawIndexed(6, 0, 0);
}

/// <summary>
/// space�L�[�ŃN���A��ʂ���V�[���J��
/// </summary>
void TransitClearScene() {
    int InputKey = KeyInputTriggerSense();
    if (InputKey & KEY_SPACE) {
        mNextScene = eScene::TITLE;
    }
}


/// <summary>
/// �Q�[���N���A�V�[���̕`��
/// </summary>
void RenderClearScene() {
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
}


/// <summary>
/// �J�����̌�����ݒ肷��
/// </summary>
/// <param name="EyePosX">�ڂ�X���W</param>
/// <param name="EyePosY">�ڂ�Y���W</param>
/// <param name="EyePosZ">�ڂ�Z���W</param>
void SetViewDir(float EyePosX,float EyePosY,float EyePosZ) {

    // Initialize the view matrix       //������ύX����Ǝ��_�ύX�ł���
    XMVECTOR Eye = XMVectorSet(EyePosX, EyePosY, EyePosZ, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose(g_View);
    g_pImmediateContext->UpdateSubresource(g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0);
}

/// <summary>
/// �Q�[���N���A�̔���
/// </summary>
void JudgeGameClear() {
    if (StageGimmick[StageSize * duckY + duckX] == 1) {
        GameClearFlag = true;
    }
    for (int i = 0; i < 32; i++) {
        if (StageGimmick[i] == 2) {
            if (JudgeStandMap[i] == 0) {
                GameClearFlag = false;
            }
        }
    }
}

/// <summary>
/// �Q�[���V�[���V�[�����瑼�̃V�[���ւ̑J��
/// </summary>
/// <param name="ClearedTime">�N���A�����u�Ԃ̎���</param>
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
/// �Q�[���V�[���̏��������s��
/// </summary>
void InitializeGameScene() {
    GameClearFlag = false;
    DuckActionMenu = KEY_1;
    CharacterDirection = 100000;
    duckX = 0;
    duckY = StageSize - 1;
    for (int i = 0; i < 12; i++)DuckActionMain[i] = 0;
    for (int i = 0; i < 8; i++) DuckActionPattern1[i] = 0;
    mainCount = 0;
    pattern1Count = 0;
    PlayOnceFlag = true;
    for (int i = 0; i < 36; i++) {
        JudgeStandMap[i] = 0;
    }
}

/// <summary>
/// �Q�[��UI�̕`��
/// </summary>
void RenderGameUI() {
    // Set vertex buffer
    UINT stride5 = sizeof(SimpleVertex);
    UINT offset5 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBufferGameUI, &stride5, &offset5);

    CBChangesEveryFrame cb;

    for (int wid = 0; wid < 6; wid++) {
        for (int hig = 0; hig < 2; hig++) {
            g_World = XMMatrixTranslation((wid - 4.5f) * 1.1f * 2 - 0.3f, -4 - hig * 1.1f * 2, -3) * XMMatrixRotationX(atan2(2, 6));
            cb.mWorld = XMMatrixTranspose(g_World);
            cb.vMeshColor = g_vMeshColor;
            cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -1.0f, 1.0f);
            cb.vLightColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
            float change = 1.0f;
            if (DuckActionMenu & KEY_1) {
                change = (sinf(time * 5.0f) + 15.0f) * 0.0625f;
            }
            cb.vChangeColor = XMFLOAT4(change, change, change, 1.0f);
            cb.vMeshColor.y = 0.85f;
            cb.vMeshColor.z = 0.8f;
            g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
            int key = DuckActionMain[hig * 6 + wid];
            if (key & KEY_UP) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVForward);
            else if (key & KEY_LEFT)  g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVLeft);
            else if (key & KEY_RIGHT) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVRight);
            else if (key & KEY_SPACE) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVJump);
            else if (key & KEY_E) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVPattern);
            else {
                g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
            }

            //
            // Render the cube
            //
            g_pImmediateContext->VSSetShader(g_pVertexShader4, NULL, 0);
            g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
            g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
            g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);
            g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->DrawIndexed(6, 0, 0);
        }
    }


    for (int wid = 0; wid < 4; wid++) {
        for (int hig = 0; hig < 2; hig++) {
            g_World = XMMatrixTranslation((wid + 1.5f) * 1.1f * 2 + 0.3f, -4 - hig * 1.1f * 2, -3) * XMMatrixRotationX(atan2(2, 6));

            cb.mWorld = XMMatrixTranspose(g_World);
            cb.vMeshColor = g_vMeshColor;
            cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -1.0f, 1.0f);
            cb.vLightColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
            cb.vChangeColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            float change = 1.0f;
            if (DuckActionMenu & KEY_2) {
                change = (sinf(time * 5.0f) + 15.0f) * 0.0625f;
            }
            cb.vChangeColor = XMFLOAT4(change, change, change, 1.0f);
            cb.vMeshColor.x = 0.7f;
            cb.vMeshColor.z = 0.8f;
            g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

            //
            // Render the cube
            //
            int key = DuckActionPattern1[hig * 4 + wid];
            if (key & KEY_UP) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVForward);
            else if (key & KEY_LEFT)  g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVLeft);
            else if (key & KEY_RIGHT) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVRight);
            else if (key & KEY_SPACE) g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVJump);
            else {
                g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
            }
            g_pImmediateContext->VSSetShader(g_pVertexShader4, NULL, 0);
            g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
            g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
            g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);
            g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->DrawIndexed(6, 0, 0);
        }
    }
}

/// <summary>
/// �I����ʂ̕`��
/// </summary>
void RenderSelectScene() {
    SetViewDir(0.0f, 1.0f, -6.0f);
    //�I����ʂ�UI�̕\��
    // Set vertex buffer
    UINT stride6 = sizeof(SimpleVertex);
    UINT offset6 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBufferSelectUI, &stride6, &offset6);

    CBChangesEveryFrame cb;

    for (int wid = 0; wid < 6; wid++) {
        for (int hig = 0; hig < 2; hig++) {
            g_World = XMMatrixTranslation((wid - 2.5f) * 3.5f,  - hig * 1.1f * 4, -3);
            cb.mWorld = XMMatrixTranspose(g_World);
            cb.vMeshColor = g_vMeshColor;
            cb.vLightDir = XMFLOAT4(0.0f, 1.0f, -1.0f, 1.0f);
            cb.vLightColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
            float change = 1.0f;
            if (SelectNum == hig * 6 + wid) {
                change = (sinf(time * 5.0f) + 15.0f) * 0.0625f;
            }
            cb.vChangeColor = XMFLOAT4(change, change, change, 1.0f);
            cb.vMeshColor.y = 0.85f;
            cb.vMeshColor.z = 0.8f;
            g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
            int key = DuckActionMain[hig * 6 + wid];
            g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

            //
            // Render the cube
            //
            g_pImmediateContext->VSSetShader(g_pVertexShader4, NULL, 0);
            g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
            g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
            g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->PSSetShader(g_pPixelShaderGoal, NULL, 0);
            g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
            g_pImmediateContext->DrawIndexed(6, 0, 0);
        }
    }

    //�w�i�̕����ɏ��������Ƃ��Ɏg�����Ǝv���Ă�
    // Set vertex buffer
    UINT stride4 = sizeof(SimpleVertex);
    UINT offset4 = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer4, &stride4, &offset4);

    g_World = XMMatrixTranslation(0, 1, 0);

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
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRVSelect);
    g_pImmediateContext->DrawIndexed(6, 0, 0);
}

/// <summary>
/// �I����ʂɂ�����L�[���͂ɂ�鑀��
/// </summary>
void KeyInputSelectScene() {
    int InputKey = KeyInputTriggerSense();
    if (InputKey & KEY_SPACE) {
        mNextScene = eScene::GAME;
    }
    else if (InputKey & KEY_ESC) {
        mNextScene = eScene::TITLE;
    }
    if (InputKey & KEY_RIGHTARROW) {
        if (SelectNum < 2) SelectNum++;
    }
    if (InputKey & KEY_LEFTARROW) {
        if (SelectNum > 0) SelectNum--;
    }
}

/// <summary>
/// �X�e�[�W�̑I��
/// </summary>
void SelectStage() {
    //�e�L�X�g�f�[�^����}�b�v�̐��������s�����Ƃ��l����ƎQ�Ƃł͂Ȃ��A�R�s�[���������Ǝv���B
    for (int i = 0; i < 36; i++) {
        StageLevel[i] = StageLevelDatas[SelectNum][i];
        StageGimmick[i] = StageGimmickDatas[SelectNum][i];
    }
}