
#include "CGVertexExt.h"


// Vertex input descriptor based on CGVertexExt
static const D3D11_INPUT_ELEMENT_DESC extVertexDesc[] = {

	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"DIFFUSE", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SPECULAR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
};


HRESULT CGVertexExt::createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout) {

	return device->CreateInputLayout(extVertexDesc, ARRAYSIZE(extVertexDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), layout);
}