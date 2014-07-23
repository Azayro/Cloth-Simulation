
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "CGBaseModel.h"


class CGPyramid : public CGBaseModel {

public:

	CGPyramid(ID3D11Device *device, ID3DBlob *vsBytecode);

	void render(ID3D11DeviceContext *context);
};

