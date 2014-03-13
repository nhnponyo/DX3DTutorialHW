// Light2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Light2.h"

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE9      g_pTexture1 = NULL; // Our texture1
LPDIRECT3DTEXTURE9      g_pTexture2 = NULL; // Our texture2

LPD3DXMESH          g_pMesh = NULL; // Our mesh object in sysmem
D3DMATERIAL9*       g_pMeshMaterials = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL; // Textures for our mesh

DWORD               g_dwNumMaterials = 0L;   // Number of mesh materials

bool TimeCheck = true;

// A structure for our custom vertex type. We added a normal, and omitted the
// color (which is provided by the material)

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	D3DXVECTOR3 normal;
	D3DCOLOR color;    // The color
#ifndef SHOW_HOW_TO_USE_TCI
	FLOAT tu, tv;   // The texture coordinates
#endif
};

// Our custom FVF, which describes our custom vertex structure
#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)
#else
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Create the D3D object.
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// Set up the structure used to create the D3DDevice. Since we are now
	// using more complex geometry, we will create a device with a zbuffer.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// Create the D3DDevice
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// Turn off culling
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	// Turn on the zbuffer
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"ponyo.bmp", &g_pTexture1)))
	{
		// If texture is not in current folder, try parent folder
		if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"..//ponyo.bmp", &g_pTexture1)))
		{
			MessageBox(NULL, L"Could not find ponyo.bmp", L"Light2.exe", MB_OK);
			return E_FAIL;
		}
	}

	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"mercury.bmp", &g_pTexture2)))
	{
		// If texture is not in current folder, try parent folder
		if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"..//mercury.bmp", &g_pTexture2)))
		{
			MessageBox(NULL, L"Could not find mercury.bmp", L"Light2.exe", MB_OK);
			return E_FAIL;
		}
	}

	// Create the vertex buffer.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(50 * 2 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Fill the vertex buffer. We are setting the tu and tv texture
	// coordinates, which range from 0.0 to 1.0
	CUSTOMVERTEX* pVertices;
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	for (DWORD i = 0; i < 50; i++)
	{
		FLOAT theta = (2 * D3DX_PI * i) / (50 - 1);

		pVertices[2 * i + 0].position = D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		pVertices[2 * i + 0].color = 0xffffffff;
		pVertices[2 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
	
#ifndef SHOW_HOW_TO_USE_TCI
		pVertices[2 * i + 0].tu = ((FLOAT)i) / (50 - 1);
		pVertices[2 * i + 0].tv = 1.0f;
#endif

		pVertices[2 * i + 1].position = D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		pVertices[2 * i + 1].color = 0xff808080;
		pVertices[2 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
#ifndef SHOW_HOW_TO_USE_TCI
		pVertices[2 * i + 1].tu = ((FLOAT)i) / (50 - 1);
		pVertices[2 * i + 1].tv = 0.0f;
		
#endif
	}
	g_pVB->Unlock();


	//tiger load
	LPD3DXBUFFER pD3DXMtrlBuffer;

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(L"Tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
		&g_pMesh)))
	{
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromX(L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
			&g_pMesh)))
		{
			MessageBox(NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK);
			return E_FAIL;
		}
	}

	// 메쉬가 버텍스 포멧으로 D3DFVF_NORMAL을 가지고 있는가?
	if ( !( g_pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		//가지고 있지 않다면 메쉬를 복제하고 D3DFVF_NORMAL을 추가한다.
		ID3DXMesh* pTempMesh = 0;
		g_pMesh->CloneMeshFVF(
			D3DXMESH_MANAGED,
			g_pMesh->GetFVF() | D3DFVF_NORMAL,  //이곳에 추가
			g_pd3dDevice,
			&pTempMesh );

		// 법선을 계산한다.
		D3DXComputeNormals( pTempMesh, 0 );

		g_pMesh->Release(); // 기존메쉬를 제거한다
		g_pMesh = pTempMesh; // 기존메쉬를 법선이 계산된 메쉬로 지정한다.
	}


	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if (g_pMeshMaterials == NULL)
		return E_OUTOFMEMORY;
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if (g_pMeshTextures == NULL)
		return E_OUTOFMEMORY;

	for (DWORD i = 0; i < g_dwNumMaterials; i++)
	{
		// Copy the material
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s(strTexture, MAX_PATH, strPrefix);
				strcat_s(strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
					strTexture,
					&g_pMeshTextures[i])))
				{
					MessageBox(NULL, L"Could not find texture map", L"Meshes.exe", MB_OK);
				}
			}
		}
	}

	// Done with the material buffer
	pD3DXMtrlBuffer->Release();

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pTexture1 != NULL)
		g_pTexture1->Release();

	if (g_pTexture2 != NULL)
		g_pTexture2->Release();

	if (g_pMeshMaterials != NULL)
		delete[] g_pMeshMaterials;

	if (g_pMeshTextures)
	{
		for (DWORD i = 0; i < g_dwNumMaterials; i++)
		{
			if (g_pMeshTextures[i])
				g_pMeshTextures[i]->Release();
		}
		delete[] g_pMeshTextures;
	}
	if (g_pMesh != NULL)
		g_pMesh->Release();

	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
	// Set up world matrix
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	//D3DXMatrixRotationX(&matWorld, 1.0f);//timeGetTime() / 500.0f);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the
	// origin, and define "up" to be in the y-direction.
	D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 3, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}




//-----------------------------------------------------------------------------
// Name: SetupLights()
// Desc: Sets up the Lights and materials for the scene.
//-----------------------------------------------------------------------------
VOID SetupLights()
{
	// Set up a material. The material here just has the diffuse and ambient
	// colors set to yellow. Note that only one material can be used at a time.
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 0.5f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 0.5f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.5f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial(&mtrl);

	// Set up a white, directional light, with an oscillating direction.
	// Note that many Lights may be active at a time (but each one slows down
	// the rendering of our scene). However, here we are just using one. Also,
	// we need to set the D3DRS_LIGHTING renderstate to enable lighting
	D3DXVECTOR3 vecDir;
	D3DXVECTOR3 vecDir2;
	D3DLIGHT9 light;
	D3DLIGHT9 light2;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	ZeroMemory(&light2, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 0.0f;
	light.Diffuse.b = 0.0f;
	light2.Type = D3DLIGHT_DIRECTIONAL;
	light2.Diffuse.r = 0.0f;
	light2.Diffuse.g = 1.0f;
	light2.Diffuse.b = 1.0f;
	vecDir = D3DXVECTOR3(1.0f,0.0f,0.0f);
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	vecDir2 = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	D3DXVec3Normalize((D3DXVECTOR3*)&light2.Direction, &vecDir2);

	light.Range = 1000.0f;
	light2.Range = 1000.0f;

	g_pd3dDevice->SetLight(0, &light);
	//g_pd3dDevice->LightEnable(0, TRUE);
	g_pd3dDevice->LightEnable(0, TimeCheck);

	g_pd3dDevice->SetLight(1, &light2);
	//g_pd3dDevice->LightEnable(1, TRUE);
	g_pd3dDevice->LightEnable(1, !TimeCheck);

	/*
	float TimeGot = timeGetTime() % 500;

	if (TimeGot <= 250)
	{
		g_pd3dDevice->LightEnable(0, TRUE);
		g_pd3dDevice->LightEnable(1, FALSE);
	}
	else
	{
		g_pd3dDevice->LightEnable(0, FALSE);
		g_pd3dDevice->LightEnable(1, TRUE);
	}
	*/ //1번째로 했을때 -> 2번째엔 SetTimer 발동


	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// Finally, turn on some ambient light.
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xFFAAAAAA);
}




//-----------------------------------------------------------------------------
// Name: Render()
//-----------------------------------------------------------------------------
VOID Render()
{
	// Clear the backbuffer and the zbuffer
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
						 D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

	// Begin the scene
	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		// Setup the Lights and materials
		SetupLights();

		// Setup the world, view, and projection matrices
		SetupMatrices();

		if ( TimeCheck )
		{
			g_pd3dDevice->SetTexture( 0, g_pTexture1 );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
		}
		else
		{
			g_pd3dDevice->SetTexture( 0, g_pTexture2 );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
		}


#ifdef SHOW_HOW_TO_USE_TCI
		// Note: to use D3D texture coordinate generation, use the stage state
		// D3DTSS_TEXCOORDINDEX, as shown below. In this example, we are using
		// the position of the vertex in camera space (D3DTSS_TCI_CAMERASPACEPOSITION)
		// to generate texture coordinates. Camera space is the vertex position
		// multiplied by the World and View matrices.  The tex coord index (TCI)  
		// parameters are passed into a texture transform, which is a 4x4 matrix  
		// which transforms the x,y,z TCI coordinates into tu, tv texture coordinates.

		// In this example, the texture matrix is setup to transform the input
		// camera space coordinates (all of R^3) to projection space (-1,+1) 
		// and finally to texture space (0,1).
		//    CameraSpace.xyzw = (input vertex position) * (WorldView)
		//    ProjSpace.xyzw = CameraSpace.xyzw * Projection           //move to -1 to 1
		//    TexSpace.xyzw = ProjSpace.xyzw * ( 0.5, -0.5, 1.0, 1.0 ) //scale to -0.5 to 0.5 (flip y)
		//    TexSpace.xyzw += ( 0.5, 0.5, 0.0, 0.0 )                  //shift to 0 to 1

		// Setting D3DTSS_TEXTURETRANSFORMFLAGS to D3DTTFF_COUNT4 | D3DTTFF_PROJECTED
		// tells D3D to divide the input texture coordinates by the 4th (w) component.
		// This divide is necessary when performing a perspective projection since
		// the TexSpace.xy coordinates prior to the homogeneous divide are not actually 
		// in the 0 to 1 range.
		D3DXMATRIXA16 mTextureTransform;
		D3DXMATRIXA16 mProj;
		D3DXMATRIXA16 mTrans;
		D3DXMATRIXA16 mScale;

		g_pd3dDevice->GetTransform( D3DTS_PROJECTION, &mProj );
		D3DXMatrixTranslation( &mTrans, 0.5f, 0.5f, 0.0f );
		D3DXMatrixScaling( &mScale, 0.5f, -0.5f, 1.0f );
		mTextureTransform = mProj * mScale * mTrans;

		g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mTextureTransform );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4 | D3DTTFF_PROJECTED );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
#endif

		// Render the vertex buffer contents
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2 );


		D3DXMATRIXA16 matWorld;
		D3DXMATRIXA16 rotMat;
		D3DXMATRIXA16 transMat;

		D3DXMatrixTranslation( &transMat, 2, 0, 0 );
		D3DXMatrixRotationY( &rotMat, timeGetTime() / 1000.0f );

		D3DXMatrixMultiply( &matWorld, &transMat, &rotMat );

		g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

		for ( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			// Set the material and texture for this subset
			g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
			g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

			// Draw the mesh subset
			g_pMesh->DrawSubset( i );
		}


		D3DXMATRIXA16 rotMat2;
		D3DXMatrixRotationY( &rotMat2, D3DX_PI );

		D3DXMatrixMultiply( &matWorld, &transMat, &rotMat );
		D3DXMatrixMultiply( &matWorld, &matWorld, &rotMat2 );
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

		for ( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			// Set the material and texture for this subset
			g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
			g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

			// Draw the mesh subset
			g_pMesh->DrawSubset( i );
		}


		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		TimeCheck = !TimeCheck;
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
	UNREFERENCED_PARAMETER(hInst);

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 04: Lights",
		WS_OVERLAPPEDWINDOW, 100, 100, 500, 500,
		NULL, NULL, wc.hInstance, NULL);
	SetTimer(hWnd, 337, 500, NULL);

	// Initialize Direct3D
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		// Create the geometry
		if (SUCCEEDED(InitGeometry()))
		{
			// Show the window
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			// Enter the message loop
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					Render();
			}
		}
	}

	UnregisterClass(L"D3D Tutorial", wc.hInstance);
	return 0;
}
