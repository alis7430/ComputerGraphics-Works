#pragma once

#include"d3dfont.h"
#include<d3dx9.h>

class Interface
{
public:
	Interface(IDirect3DDevice9* device);
	~Interface();

	bool render(D3DCOLOR color, float timeDelta, D3DXVECTOR3* Pos, int* score);
	float GetTime() { return _time; }
private:
	IDirect3DDevice9* _device;

	CD3DFont* _font;

	D3DXVECTOR3 _Pos;
	int		  _score;
	float	  _time;
	float     _timeElapsed;
	char      _PosString[14];
	char	  _scoreString[25];
	char	  _TimeRemain[18];
};
