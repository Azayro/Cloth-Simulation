
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "CGBaseModel.h"


class CGCube : public CGBaseModel {

	// augment basic CGBaseModel with texture view
	ID3D11ShaderResourceView			*textureResourceView;
	ID3D11SamplerState					*sampler;

public:

	CGCube(ID3D11Device *device, ID3DBlob *vsBytecode, ID3D11ShaderResourceView *tex_view, ID3D11SamplerState *_sampler);
	~CGCube();

	void render(ID3D11DeviceContext *context);
};

