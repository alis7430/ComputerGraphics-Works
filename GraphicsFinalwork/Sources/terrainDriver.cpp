#include "d3dUtility.h"
#include "terrain.h"
#include "psystem.h"
#include "camera.h"
#include "fps.h"
#include "Interface.h"

#include <cstdlib>
#include <ctime>


// Globals

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;

Terrain* TheTerrain = 0;
psys::PSystem* Sno = 0;

Camera   TheCamera(Camera::LANDOBJECT);

FPSCounter* FPS = 0;
Interface* iface = 0;

ID3DXMesh* Teapot = 0;
ID3DXMesh* Sphere = 0;

D3DXMATRIX World;
d3d::BoundingSphere BSphere;

bool isClicked;

void SetRandomPos(Terrain* T)
{
	float x = d3d::GetRandomFloat(0.0f, 300.0f);
	float z = d3d::GetRandomFloat(0.0f, 300.0f);
	float y = T->getHeight(x, z);

	D3DXMatrixTranslation(&World, x ,y + 5.0f, z);
	BSphere._center = D3DXVECTOR3(x, y + 5.0f, z);

	isClicked = false;
}

// Framework Functions
d3d::Ray CalcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	D3DVIEWPORT9 vp;
	Device->GetViewport(&vp);

	D3DXMATRIX proj;
	Device->GetTransform(D3DTS_PROJECTION, &proj);

	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	d3d::Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);

	return ray;
}

void TransformRay(d3d::Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

bool RaySphereIntTest(d3d::Ray* ray, d3d::BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;

	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);

	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);

	// test for imaginary number
	if (discriminant < 0.0f)
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;

	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;

	return false;
}
bool Setup()
{
	//
	// Create the terrain.
	//

	D3DXVECTOR3 lightDirection(0.0f, 1.0f, 0.0f);
	TheTerrain = new Terrain(Device, "coastMountain64.raw", 64, 64, 10, 0.5f);
	TheTerrain->genTexture(&lightDirection);

	// Create the snow.

	srand((unsigned int)time(0));


	d3d::BoundingBox boundingBox;
	boundingBox._min = D3DXVECTOR3(-300.0f, 0.0f, -300.0f);
	boundingBox._max = D3DXVECTOR3(300.0f, 155.0f, 300.0f);
	Sno = new psys::Snow(&boundingBox, 5000);
	Sno->init(Device, "snowflake.dds");

	// Create the font.

	FPS = new FPSCounter(Device);
	iface = new Interface(Device);

	//Create the teapot.
	D3DXCreateTeapot(Device, &Teapot, 0);

	//Set TeapotPos
	D3DXMatrixTranslation(&World, 0.0f, 50.0f, 10.0f);
	BSphere._center = D3DXVECTOR3(0.0f, 50.0f, 10.0f);

	//
	// Compute the bounding sphere.
	//

	BYTE* v = 0;
	Teapot->LockVertexBuffer(0, (void**)&v);

	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		Teapot->GetNumVertices(),
		D3DXGetFVFVertexSize(Teapot->GetFVF()),
		&BSphere._center,
		&BSphere._radius);

	Teapot->UnlockVertexBuffer();

	//
	// Build a sphere mesh that describes the teapot's bounding sphere.
	//

	D3DXCreateSphere(Device, BSphere._radius, 20, 20, &Sphere, 0);

	//
	// Set light.
	//

	D3DXVECTOR3 dir(0.707f, -0.0f, 0.707f);
	D3DXCOLOR col(1.0f, 1.0f, 1.0f, 1.0f);
	D3DLIGHT9 light = d3d::InitDirectionalLight(&dir, &col);

	Device->SetLight(0, &light);
	Device->LightEnable(0, true);
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, false);

	//
	// Set texture filters.
	//

	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);


	// Set projection matrix.

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.25f, // 45 - degree
			(float)Width / (float)Height,
			1.0f,
			1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);


	isClicked = false;

	return true;
}

void Cleanup()
{
	d3d::Delete<Terrain*>(TheTerrain);
	d3d::Delete<FPSCounter*>(FPS);
	d3d::Delete<Interface*>(iface);
	d3d::Delete<psys::PSystem*>(Sno);
	d3d::Release<ID3DXMesh*>(Teapot);
	d3d::Release<ID3DXMesh*>(Sphere);
}

bool Display(float timeDelta)
{
	// Update the scene:


	if( Device )
	{
		if( ::GetAsyncKeyState(VK_UP) & 0x8000f )
			TheCamera.walk(100.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_DOWN) & 0x8000f )
			TheCamera.walk(-100.0f * timeDelta);

		if( ::GetAsyncKeyState('A') & 0x8000f )
			TheCamera.yaw(-2.0f * timeDelta);
		
		if( ::GetAsyncKeyState('D') & 0x8000f )
			TheCamera.yaw(2.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_LEFT) & 0x8000f )
			TheCamera.strafe(-100.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_RIGHT) & 0x8000f )
			TheCamera.strafe(100.0f * timeDelta);

		if( ::GetAsyncKeyState('S') & 0x8000f )
			TheCamera.pitch(2.0f * timeDelta);

		if( ::GetAsyncKeyState('W') & 0x8000f )
			TheCamera.pitch(-2.0f * timeDelta);


		// Set TeapotPos
		if (isClicked)
			SetRandomPos(TheTerrain);
		
		//Set CameraPos

		D3DXVECTOR3 pos;
		TheCamera.getPosition(&pos);
		if (pos.x < 300.0f && pos.z < 300.0f)
		{
			float height = TheTerrain->getHeight(pos.x, pos.z);
			pos.y = height + 5.0f; // add height because we're standing up
		}
		TheCamera.setPosition(&pos);

		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);

		Sno->update(timeDelta);

		// Draw the scene:


		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);
		Device->BeginScene();

		D3DXMATRIX I;
		D3DXMatrixIdentity(&I);


		if( TheTerrain )
			TheTerrain->draw(&I, false);

		D3DXMATRIX Y;
		D3DXMatrixIdentity(&Y);
		Device->SetTransform(D3DTS_WORLD, &Y);
		Sno->render();
		
		if (FPS)
			FPS->render(0xffffffff, timeDelta);
		if (iface)
			iface->render(0xffffffff, timeDelta, &pos);

		// Render the teapot.
		Device->SetTransform(D3DTS_WORLD, &World);
		Device->SetMaterial(&d3d::YELLOW_MTRL);
		Teapot->DrawSubset(0);

		// Render the bounding sphere with alpha blending so we can see 
		// through it.

		Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		D3DMATERIAL9 blue = d3d::BLUE_MTRL;
		blue.Diffuse.a = 0.25f; // 25% opacity
		Device->SetMaterial(&blue);
		Sphere->DrawSubset(0);

		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);


		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	case WM_LBUTTONDOWN:

		// compute the ray in view space given the clicked screen point
		d3d::Ray ray = CalcPickingRay(LOWORD(lParam), HIWORD(lParam));

		// transform the ray to world space
		D3DXMATRIX view;
		Device->GetTransform(D3DTS_VIEW, &view);

		D3DXMATRIX viewInverse;
		D3DXMatrixInverse(&viewInverse, 0, &view);

		TransformRay(&ray, &viewInverse);

		// test for a hit
		if (RaySphereIntTest(&ray, &BSphere))
		{
			::MessageBox(0, "Hit!", "HIT", 0);
			isClicked = true;
		}

		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

// WinMain
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}