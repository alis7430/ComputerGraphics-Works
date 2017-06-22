#include"Interface.h"
#include <cstdio>

Interface :: Interface(IDirect3DDevice9* device)
{
	_device = device;
	_font = new CD3DFont("Terminal", 15, 0);
	_font->InitDeviceObjects(_device);
	_font->RestoreDeviceObjects();

	_timeElapsed = 0.0f;
}

Interface :: ~Interface()
{
	if (_font)
	{
		_font->InvalidateDeviceObjects();
		_font->DeleteDeviceObjects();
		delete _font;
	}
}

bool Interface::render(D3DCOLOR color, float timeDelta, D3DXVECTOR3* Pos)
{
	if (_font)
	{
		_Pos = *Pos;
		int x = (int)_Pos.x;
		int y = (int)_Pos.y;
		int z = (int)_Pos.z;
		_timeElapsed += timeDelta;
		if (_timeElapsed >= 1.0f)
		{
			sprintf(_PosString, "(%d,%d,%d)", x, y, z);
			_PosString[14] = '\0';
			_timeElapsed = 0.0f;
		}
		_font->DrawText(20, 60, color, _PosString);
	}
	return true;
}