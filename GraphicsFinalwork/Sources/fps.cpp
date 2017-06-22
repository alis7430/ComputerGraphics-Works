#include "fps.h"
#include <cstdio>

FPSCounter::FPSCounter(IDirect3DDevice9* device)
{
	_device = device;

	_font = new CD3DFont("ARBONNIE", 15, 0);
	_font->InitDeviceObjects( _device );
	_font->RestoreDeviceObjects();

	_frameCnt = 0;
	_timeElapsed = 0.0f;
	_fps         = 0.0f;
}

FPSCounter::~FPSCounter()
{
	if( _font )
	{
		_font->InvalidateDeviceObjects();
		_font->DeleteDeviceObjects();
		delete _font;
	}
}

bool FPSCounter::render(D3DCOLOR color, float timeDelta)
{
	if( _font )
	{
		_frameCnt++;

		_timeElapsed += timeDelta;

		if(_timeElapsed >= 1.0f)
		{
			_fps = (float)_frameCnt / _timeElapsed;

			sprintf(_fpsString, "FPS : %f", _fps);
			_fpsString[8] = '\0'; 

			_timeElapsed = 0.0f;
			_frameCnt    = 0;
		}

		_font->DrawText(20, 20, color, _fpsString);	
	}
	return true;
}