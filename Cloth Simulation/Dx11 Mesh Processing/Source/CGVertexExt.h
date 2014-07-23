
#pragma once

#include <D3DX11.h>
#include <xnamath.h>

// Vertex structure - our 3D models will store this information per-vertex
struct CGVertexExt  {

	XMFLOAT3			pos;
	XMFLOAT3			normal;
	XMCOLOR				matDiffuse;
	XMCOLOR				matSpecular;
	XMFLOAT2			texCoord;

	static HRESULT createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout);
};
