
#pragma once

#include <D3DX11.h>
#include <xnamath.h>

// Basic vertex structure
struct CGVertexBasic  {

	XMFLOAT3			pos;
	XMCOLOR				colour;

	static HRESULT createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout);
};

