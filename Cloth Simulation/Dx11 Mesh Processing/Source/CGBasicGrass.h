
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "CGBaseModel.h"


//
// Grass vertex structure
//

struct CGBasicGrassVertex  {

	XMFLOAT3			pos;
	XMCOLOR				colour;
	FLOAT				weight;

	static HRESULT createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout);
};


class CGBasicGrass : public CGBaseModel {

	DWORD				numBlades;

public:

	CGBasicGrass(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD numPoints, DWORD terrainWidth, DWORD terrainHeight);

	void render(ID3D11DeviceContext *context);
};

