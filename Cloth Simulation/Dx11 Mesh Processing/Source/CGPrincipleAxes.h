
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "CGVertexBasic.h"
#include "CGBaseModel.h"

class CGPrincipleAxes : public CGBaseModel {

public:

	CGPrincipleAxes(ID3D11Device *device, ID3DBlob *vsBytecode);

	void render(ID3D11DeviceContext *context);
};
