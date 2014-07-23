
#pragma once

#include <D3DX11.h>
#include <xnamath.h>


// Abstract base class to model mesh objects for rendering in DirectX
class CGBaseModel {

protected:

	ID3D11Buffer					*vertexBuffer;
	ID3D11Buffer					*indexBuffer;
	ID3D11InputLayout				*inputLayout;

public:

	CGBaseModel();
	virtual ~CGBaseModel();

	virtual void render(ID3D11DeviceContext *context) = 0;
};
