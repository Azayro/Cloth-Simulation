
#pragma once

#include <D3DX11.h>
#include <xnamath.h>

class CGBaseModel;

class CGModelInstance {

	XMFLOAT3					T; // model position
	XMFLOAT3					E; // rotation angles
	CGBaseModel					*model; // weak reference to mesh model
	
public:

	CGModelInstance();
	CGModelInstance(CGBaseModel *_model, const XMFLOAT3& initT, const XMFLOAT3& initE);

	void translate(const XMFLOAT3& dT);
	void rotate(const XMFLOAT3& dE);
	void setupCBuffer(ID3D11DeviceContext *context, ID3D11Buffer *cbuffer);
	void render(ID3D11DeviceContext *context);
};
