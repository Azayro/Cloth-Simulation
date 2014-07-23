
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "CGBaseModel.h"


class CGBasicTerrain : public CGBaseModel {

	DWORD					w, h; // dimensions of the terrain on the (x, z) plane

public:

	CGBasicTerrain(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD newTerrainWidth, DWORD newTerrainHeight);

	void render(ID3D11DeviceContext *context);
};
