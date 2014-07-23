
#include "HermiteCurve.h"
#include <iostream>
#include "CGVertexExt.h"

using namespace std;


#pragma region Curve model data

// Vertices define the control points of the curve that are to be interpolated

static const CGVertexExt controlPoints[] = {

	{XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(1.25f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.3f, 0.0f)},
	{XMFLOAT3(2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(0.0f, 1.0f, 1.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.6f, 0.0f)},
	{XMFLOAT3(3.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMCOLOR(1.0f, 0.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},
	{XMFLOAT3(4.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(1.0f, 0.0f, 0.0f, 1.0f), XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)}
};


// Model indices with adjacency.  Each line has v0 - v1 - v2 - v3 where v1 - v2 forms the primitive (v1 being the leading vertex) and v0 an v3 are the adjacent vertices.  In this example vertex 0 and vertex 5 are 'null' vertices ie. not part of the actual curve - just included to aid calculation of tangents are ends of (open) curve.

static const DWORD curveAdjIndices[] = {
	
	0, 1, 2, 3,
	1, 2, 3, 4,
	2, 3, 4, 5
};


#pragma endregion


HermiteCurve::HermiteCurve(ID3D11Device *device, ID3DBlob *vsBytecode) {

	try
	{
		// Setup Hermite curve control point (vertex) buffer

		if (!device || !vsBytecode)
			throw("Invalid parameters for Hermite curve model instantiation");

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGVertexExt) * 6;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = controlPoints;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");


		// Setup index buffer
		D3D11_BUFFER_DESC indexDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));

		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(DWORD) * 3 * 4; // 3 segments, 4 indices per segment
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexData.pSysMem = curveAdjIndices;

		hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);

		if (!SUCCEEDED(hr))
			throw("Index buffer cannot be created");


		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexExt::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");
	}
	catch(char *err)
	{
		cout << "Hermite curve object could not be instantiated due to:\n";
		cout << err << endl << endl;
		
		if (vertexBuffer)
			vertexBuffer->Release();

		if (indexBuffer)
			indexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		inputLayout = nullptr;
	}
}


void HermiteCurve::render(ID3D11DeviceContext *context) {

	// Validate object before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer ||  !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGVertexExt)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);

	// Draw curve object - Geometry shader on (assumed) current pipeline will perform the interpolation of the curve
	context->DrawIndexed(12, 0, 0);
}
