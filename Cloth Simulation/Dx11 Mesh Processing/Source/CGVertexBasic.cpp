
#include "CGVertexBasic.h"


// Vertex input descriptor based on CGVertexBasic
static const D3D11_INPUT_ELEMENT_DESC basicVertexDesc[] = {

	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};


HRESULT CGVertexBasic::createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout) {

	return device->CreateInputLayout(basicVertexDesc, ARRAYSIZE(basicVertexDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), layout);
}
