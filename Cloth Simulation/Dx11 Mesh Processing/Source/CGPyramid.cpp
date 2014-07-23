
#include "CGPyramid.h"
#include <iostream>
#include "CGVertexExt.h"

using namespace std;


#pragma region Pyramid model data

static const CGVertexExt pyramidVertices[] = {
	
	{XMFLOAT3(0.0f, 4.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(1.0f, 0.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(-0.577f, -0.577f, -0.577f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.577f, -0.577f, -0.577f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.577f, -0.577f, 0.577f), XMCOLOR(0.0f, 1.0f, 1.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT3(-0.577f, -0.577f, 0.577f), XMCOLOR(1.0f, 0.0f, 1.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)}
};

static const DWORD pyramidIndices[] = {
	
	0, 2, 1,
	0, 3, 2,
	0, 4, 3,
	0, 1, 4,
	1, 2, 4,
	2, 3, 4
};

#pragma endregion


CGPyramid::CGPyramid(ID3D11Device *device, ID3DBlob *vsBytecode) {

	// Setup pyramid buffers
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	inputLayout = nullptr;
	
	try
	{
		if (!device || !vsBytecode)
			throw("Invalid parameters for pyramid model instantiation");

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGVertexExt) * 5;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = pyramidVertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");


		// Setup index buffer
		D3D11_BUFFER_DESC indexDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));

		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(DWORD) * 6 * 3; // 6 faces, 3 indices per face
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexData.pSysMem = pyramidIndices;

		hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);

		if (!SUCCEEDED(hr))
			throw("Index buffer cannot be created");

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexExt::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");
	}
	catch (char *err)
	{
		cout << "Pyramid could not be instantiated due to:\n";
		cout << err << endl << endl;
		
		if (vertexBuffer)
			vertexBuffer->Release();

		if (indexBuffer)
			indexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		indexBuffer = nullptr;
		inputLayout = nullptr;
	}
}


void CGPyramid::render(ID3D11DeviceContext *context) {

	// validate pyramid before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set pyramid vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGVertexExt)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw pyramid
	context->DrawIndexed(18, 0, 0);
}
