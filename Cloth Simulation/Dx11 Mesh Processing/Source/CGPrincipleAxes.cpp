
#include "CGPrincipleAxes.h"
#include <iostream>

using namespace std;


#pragma region Principle axes model data

static const CGVertexBasic vertices[] = {
	
	// x
	{XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	{XMFLOAT3(1.0f, 0.0f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	{XMFLOAT3(1.025f, 0.025f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	{XMFLOAT3(1.075f, 0.075f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	{XMFLOAT3(1.025f, 0.075f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	{XMFLOAT3(1.075f, 0.025f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
	
	// y
	{XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	{XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	{XMFLOAT3(-0.075f, 1.075f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	{XMFLOAT3(-0.05f, 1.05f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	{XMFLOAT3(-0.025f, 1.075f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	{XMFLOAT3(-0.075f, 1.025f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
	
	// z
	{XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.0f, 0.0f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.025f, 0.075f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.075f, 0.075f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.075f, 0.075f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.025f, 0.025f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.025f, 0.025f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)},
	{XMFLOAT3(0.075f, 0.025f, 1.0f), XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f)}
};

#pragma endregion


CGPrincipleAxes::CGPrincipleAxes(ID3D11Device *device, ID3DBlob *vsBytecode) {

	try
	{
		// Setup principle axes buffers

		if (!device || !vsBytecode)
			throw("Invalid parameters for principle axes model instantiation");

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGVertexBasic) * 20;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = vertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexBasic::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");
	}
	catch(char *err)
	{
		cout << "Principle Axes object could not be instantiated due to:\n";
		cout << err << endl << endl;
		
		if (vertexBuffer)
			vertexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		inputLayout = nullptr;
	}
}


void CGPrincipleAxes::render(ID3D11DeviceContext *context) {

	// Validate object before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGVertexBasic)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Draw principle axes object
	// Note: Draw vertices in the buffer one after the other.  Not the most efficient approach (see duplication in the above vertex data)
	// This is shown here for demonstration purposes
	context->Draw(20, 0);
}
