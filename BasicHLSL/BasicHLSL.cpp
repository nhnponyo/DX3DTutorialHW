//--------------------------------------------------------------------------------------
// File: BasicHLSL.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
//#include "DXUTgui.h"
//#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
//ID3DXFont*                  g_pFont = NULL;         // Font for drawing text
//ID3DXSprite*                g_pSprite = NULL;       // Sprite for batching draw text calls
//bool                        g_bShowHelp = true;     // If true, it renders the UI control text


CModelViewerCamera          g_Camera;               // A model viewing camera 카메라
ID3DXEffect*                g_pEffect = NULL;       // D3DX effect interface 이펙터
ID3DXMesh*                  g_pMesh = NULL;         // Mesh object 메쉬
IDirect3DTexture9*          g_pMeshTexture = NULL;  // Mesh texture 텍스쳐

bool                        g_bEnablePreshader;     // preshader
D3DXMATRIXA16               g_mCenterWorld;          //월드 메트릭스

#define MAX_LIGHTS 3									//조명 갯수 3개
CDXUTDirectionWidget g_LightControl[MAX_LIGHTS];       //D3D 디렉션 위젯 
float                       g_fLightScale;				//조명 밝기
int                         g_nNumActiveLights;			//켜진 조명 갯수
int                         g_nActiveLight;				//활성화된 조명 번호


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
/*
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_ENABLE_PRESHADER    5
#define IDC_NUM_LIGHTS          6
#define IDC_NUM_LIGHTS_STATIC   7
#define IDC_ACTIVE_LIGHT        8
#define IDC_LIGHT_SCALE         9
#define IDC_LIGHT_SCALE_STATIC  10

*/
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
//bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
//void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
//void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
								  D3DFORMAT BackBufferFormat, bool bWindowed,void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, 
								 const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
						  bool* pbNoFurtherProcessing, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void InitApp();
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );

//void RenderText( double fTime );


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );  //장치를 사용할 수 있는지
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

	InitApp( );
	DXUTInit( true, true ); // Parse the command line and show msgboxes
	DXUTSetHotkeyHandling( true, true, true );  // 기본키 입력을 처리할 것인가 -> 알트엔터 전체창, esc로 나가기, pause로 멈추기

	DXUTCreateWindow( L"BasicHLSL" );
	DXUTCreateDevice( true, 640, 480 ); //창화면 전체화면 bool, size

	DXUTMainLoop(); //무한루프
	return DXUTGetExitCode( ); //종료 코드 받아오기


	// DXUTSetCallbackKeyboard( KeyboardProc );
    //DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // Show the cursor and clip it when in full screen
    //DXUTSetCursorSettings( true, true );

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.
}


//--------------------------------------------------------------------------------------
// 프로그램 초기화
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_bEnablePreshader = true; //A shader compile flag that gives the compiler hints about how the shader will be used.
								// 쉐이더가 어떻게 사용될 것인지에 대해 미리 알려서 성능을 최적화 시켜준다

	for ( int i = 0; i < MAX_LIGHTS; i++ )
	{
		g_LightControl[i].SetLightDirection( D3DXVECTOR3( sinf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ),
			0, -cosf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ) ) );
	} // 조명 최초 방향벡터 초기화

    g_nActiveLight = 0; // 인덱스
    g_nNumActiveLights = 1; //켜진 조명 갯수 초깃값
    g_fLightScale = 1.0f; //조명 최대 밝기




    // Initialize dialogs
	
//    g_SettingsDlg.Init( &g_DialogResourceManager );
 //   g_HUD.Init( &g_DialogResourceManager );
 //   g_SampleUI.Init( &g_DialogResourceManager );
	
 //   g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
   // g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    //g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    //g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
	/*
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;

    WCHAR sz[100];
    iY += 24;
    swprintf_s( sz, 100, L"# Lights: %d", g_nNumActiveLights );
    g_SampleUI.AddStatic( IDC_NUM_LIGHTS_STATIC, sz, 35, iY += 24, 125, 22 );
    g_SampleUI.AddSlider( IDC_NUM_LIGHTS, 50, iY += 24, 100, 22, 1, MAX_LIGHTS, g_nNumActiveLights );

    iY += 24;
    swprintf_s( sz, 100, L"Light scale: %0.2f", g_fLightScale );
    g_SampleUI.AddStatic( IDC_LIGHT_SCALE_STATIC, sz, 35, iY += 24, 125, 22 );
    g_SampleUI.AddSlider( IDC_LIGHT_SCALE, 50, iY += 24, 100, 22, 0, 20, ( int )( g_fLightScale * 10.0f ) );

    iY += 24;
    g_SampleUI.AddButton( IDC_ACTIVE_LIGHT, L"Change active light (K)", 35, iY += 24, 125, 22, 'K' );
    g_SampleUI.AddCheckBox( IDC_ENABLE_PRESHADER, L"Enable preshaders", 35, iY += 24, 125, 22, g_bEnablePreshader );
	*/
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//--------------------------------------------------------------------------------------
//장치를 사용할 수 있는가 확인
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;
	//쉐이더 버젼 체크

    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
/*
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}
*/

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
//디바이스를 생성한다

HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

    V_RETURN( LoadMesh( pd3dDevice, L"tiny\\tiny.x", &g_pMesh ) ); //메시를 로드해서 보관한다

    D3DXVECTOR3* pData;
    D3DXVECTOR3 vCenter;
    FLOAT fObjectRadius;
    V( g_pMesh->LockVertexBuffer( 0, ( LPVOID* )&pData ) );
	//락한후
    V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(),
                                  D3DXGetFVFVertexSize( g_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
	//메쉬 경계 산출 충돌체크와 컬링에 필요한 구를 위한 경계 산출
    V( g_pMesh->UnlockVertexBuffer() );
	//언락
	
	//월드 좌표를 중앙으로
    D3DXMatrixTranslation( &g_mCenterWorld, -vCenter.x, -vCenter.y, -vCenter.z );
    D3DXMATRIXA16 m;
    D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterWorld *= m;
    D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    g_mCenterWorld *= m;

    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );

    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].SetRadius( fObjectRadius );
		// 조명 이동 원 반경 설정

    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE; // 쉐이더의 데이터를 메모리에 보존하지 않는다. 이펙트 메모리 사용량을 50% 가량 줄인다.

#if defined( DEBUG ) || defined( _DEBUG )
    dwShaderFlags |= D3DXSHADER_DEBUG;
    #endif

#ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
#ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif


    if( !g_bEnablePreshader )
        dwShaderFlags |= D3DXSHADER_NO_PRESHADER;
	// 미리 컴파일러에 쉐이더 사용을 알리는 플래그

    // 이펙트 파일 찾기
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"BasicHLSL.fx" ) ); 

    // 파일을 찾을 수 없을 때 실제로 생성
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

    // 텍스쳐 파일 찾기
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"tiny\\tiny_skin.dds" ) );
	//실제 텍스쳐 생성
    V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT,
                                           D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                                           D3DX_DEFAULT, D3DX_DEFAULT, 0,
                                           NULL, NULL, &g_pMeshTexture ) );

    // Set effect variables as needed
    D3DXCOLOR colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
    D3DXCOLOR colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );

	//카메라 설정
    V_RETURN( g_pEffect->SetValue( "g_MaterialAmbientColor", &colorMtrlAmbient, sizeof( D3DXCOLOR ) ) );
    V_RETURN( g_pEffect->SetValue( "g_MaterialDiffuseColor", &colorMtrlDiffuse, sizeof( D3DXCOLOR ) ) );
    V_RETURN( g_pEffect->SetTexture( "g_MeshTexture", g_pMeshTexture ) );

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -15.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

	//카메라가 원형으로
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

    return S_OK;
}

//--------------------------------------------------------------------------------------
//메쉬 로딩과 최적화
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh )
{
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) ); //FX파일 찾기
    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh ) );

    DWORD* rgdwAdjacency = NULL;

    if( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
    {
        ID3DXMesh* pTempMesh;
        V( pMesh->CloneMeshFVF( pMesh->GetOptions(),
                                pMesh->GetFVF() | D3DFVF_NORMAL, // 메쉬 안에 노말벡터가 없으면 노말벡터를 생성해줌
                                pd3dDevice, &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );

        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }
  
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3]; //버텍스 가져오기 ( 면 * 3 )
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) ); //성능 최적화 시킨 메쉬의 버텍스 인접 정보를 구한다.
    V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) ); //메쉬 최적화 http://coreafive.tistory.com/201
    delete []rgdwAdjacency;

    *ppMesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// 디바이스를 잃었을때 다시 생성
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
    if( g_pEffect )
       V_RETURN( g_pEffect->OnResetDevice() );

    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].OnD3D9ResetDevice( pBackBufferSurfaceDesc );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//매 프레임마다 사전 호출. 실제 렌더링은 하지 않고 각종 object update를 한다
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
// 렌더 
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXVECTOR3 vLightDir[MAX_LIGHTS];
    D3DXCOLOR vLightDiffuse[MAX_LIGHTS];
    UINT iPass, cPasses;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    // Clear the render target and the zbuffer 
	//초기화
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
                          0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        // 카메라로부터 행렬 얻어오기
        mWorld = g_mCenterWorld * *g_Camera.GetWorldMatrix();
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mWorldViewProjection = mWorld * mView * mProj;

        // 조명 아이콘 그려주는 곳
        for( int i = 0; i < g_nNumActiveLights; i++ )
        {
            D3DXCOLOR arrowColor = ( i == g_nActiveLight ) ? D3DXCOLOR( 1, 1, 0, 1 ) : D3DXCOLOR( 1, 1, 1, 1 );
			//색상
            V( g_LightControl[i].OnRender9( arrowColor, &mView, &mProj, g_Camera.GetEyePt() ) );
			//조명 방향 설정
            vLightDir[i] = g_LightControl[i].GetLightDirection();
			//조명 색 설정
            vLightDiffuse[i] = g_fLightScale * D3DXCOLOR( 1, 1, 1, 1 );
        }

		//쉐이더 설정 시작

		//조명 방향
        V( g_pEffect->SetValue( "g_LightDir", vLightDir, sizeof( D3DXVECTOR3 ) * MAX_LIGHTS ) );
		//조명 색상
        V( g_pEffect->SetValue( "g_LightDiffuse", vLightDiffuse, sizeof( D3DXVECTOR4 ) * MAX_LIGHTS ) );

        // 월드 매트릭스 * 뷰 매트릭스 = 프로젝션 매트릭스
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
		//월드 매트릭스
        V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );
        V( g_pEffect->SetFloat( "g_fTime", ( float )fTime ) );

        D3DXCOLOR vWhite = D3DXCOLOR( 1, 1, 1, 1 );
		//조명 및 색상
        V( g_pEffect->SetValue( "g_MaterialDiffuseColor", &vWhite, sizeof( D3DXCOLOR ) ) );
        V( g_pEffect->SetFloat( "g_fTime", ( float )fTime ) );
        V( g_pEffect->SetInt( "g_nNumLights", g_nNumActiveLights ) );

        // 조명에 따라서 .fx파일에서 정의된대로 렌더
        switch( g_nNumActiveLights )
        {
            case 1:
                 V( g_pEffect->SetTechnique( "RenderSceneWithTexture1Light" ) );
				break;
            case 2:
                 V( g_pEffect->SetTechnique( "RenderSceneWithTexture2Light" ) );
				break;
            case 3:
                 V( g_pEffect->SetTechnique( "RenderSceneWithTexture3Light" ) );
				break;
        }


        // Apply the technique contained in the effect 
		// 실제 쉐이더 렌더
        V( g_pEffect->Begin( &cPasses, 0 ) );

        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

			//쉐이더에서 반환하는 테크닉 적용해 메쉬 렌더링
             V( g_pEffect->CommitChanges() );

            // Render the mesh with the applied technique
            V( g_pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
        }
        V( g_pEffect->End() );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
/*
void RenderText( double fTime )
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 2, 0 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawFormattedTextLine( L"fTime: %0.1f  sin(fTime): %0.4f", fTime, sin( fTime ) );

    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height - 15 * 6 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls:" );

        txtHelper.SetInsertionPos( 20, pd3dsdBackBuffer->Height - 15 * 5 );
        txtHelper.DrawTextLine( L"Rotate model: Left mouse button\n"
                                L"Rotate light: Right mouse button\n"
                                L"Rotate camera: Middle mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n" );

        txtHelper.SetInsertionPos( 250, pd3dsdBackBuffer->Height - 15 * 5 );
        txtHelper.DrawTextLine( L"Hide help: F1\n"
                                L"Quit: ESC\n" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
    txtHelper.End();
}
*/

//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    g_LightControl[g_nActiveLight].HandleMessages( hWnd, uMsg, wParam, lParam );
	//선택된 라이트 번호의 DirectionWidget 객체의 메세지 발생 처리

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	//좌우 클릭에 따른 카메라 메세지 발생 처리

    return 0;

	// Always allow dialog resource manager calls to handle global messages
	// so GUI state is updated correctly
	//     *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	//     if( *pbNoFurtherProcessing )
	//         return 0;
	// 
	//     if( g_SettingsDlg.IsActive() )
	//     {
	//         g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
	//         return 0;
	//     }

	// Give the dialogs a chance to handle the message first
	//     *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	//     if( *pbNoFurtherProcessing )
	//         return 0;
	//     *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
	//     if( *pbNoFurtherProcessing )
	//         return 0;

}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
/*
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
//                g_bShowHelp = !g_bShowHelp; break;
        }
    }
}
*/

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
/*
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{

    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;

        case IDC_ENABLE_PRESHADER:
        {
            g_bEnablePreshader = g_SampleUI.GetCheckBox( IDC_ENABLE_PRESHADER )->GetChecked();

            if( DXUTGetD3D9Device() != NULL )
            {
                OnLostDevice( NULL );
                OnDestroyDevice( NULL );
                OnCreateDevice( DXUTGetD3D9Device(), DXUTGetD3D9BackBufferSurfaceDesc(), NULL );
                OnResetDevice( DXUTGetD3D9Device(), DXUTGetD3D9BackBufferSurfaceDesc(), NULL );
            }
            break;
        }

        case IDC_ACTIVE_LIGHT:
            if( !g_LightControl[g_nActiveLight].IsBeingDragged() )
            {
                g_nActiveLight++;
                g_nActiveLight %= g_nNumActiveLights;
            }
            break;

        case IDC_NUM_LIGHTS:
            if( !g_LightControl[g_nActiveLight].IsBeingDragged() )
            {
                WCHAR sz[100];
                swprintf_s( sz, 100, L"# Lights: %d", g_SampleUI.GetSlider( IDC_NUM_LIGHTS )->GetValue() );
                g_SampleUI.GetStatic( IDC_NUM_LIGHTS_STATIC )->SetText( sz );

                g_nNumActiveLights = g_SampleUI.GetSlider( IDC_NUM_LIGHTS )->GetValue();
                g_nActiveLight %= g_nNumActiveLights;
            }
            break;

        case IDC_LIGHT_SCALE:
            g_fLightScale = ( float )( g_SampleUI.GetSlider( IDC_LIGHT_SCALE )->GetValue() * 0.10f );

            WCHAR sz[100];
            swprintf_s( sz, 100, L"Light scale: %0.2f", g_fLightScale );
            g_SampleUI.GetStatic( IDC_LIGHT_SCALE_STATIC )->SetText( sz );
            break;
    }

}
*/

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
//창을 관리하는 환경을 잃어버림
void CALLBACK OnLostDevice( void* pUserContext ) // device를 잃었거나, 리셋 하기 전
{
    CDXUTDirectionWidget::StaticOnD3D9LostDevice(); //장치 해지 or 리셋 할 때
    if( g_pEffect )
        g_pEffect->OnLostDevice();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext ) //리소스 해제, release device
{
    CDXUTDirectionWidget::StaticOnD3D9DestroyDevice(); //래핑된 리소스 해제
    SAFE_RELEASE( g_pEffect ); //쉐이더
    SAFE_RELEASE( g_pMesh );
    SAFE_RELEASE( g_pMeshTexture );
}



