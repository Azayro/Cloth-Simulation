
#include "CGBasicGrass.h"
#include <iostream>

using namespace std;

#pragma region CGBasicGrassVertex layout descriptor and interface setup

// Vertex input descriptor based on CGBasicGrassVertex
static const D3D11_INPUT_ELEMENT_DESC basicGrassVertexDesc[] = {

	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"WEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

HRESULT CGBasicGrassVertex::createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout) {

	return device->CreateInputLayout(basicGrassVertexDesc, ARRAYSIZE(basicGrassVertexDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), layout);
}

#pragma endregion



CGBasicGrass::CGBasicGrass(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD numPoints, DWORD terrainWidth, DWORD terrainHeight) {

	// Setup grass model buffers
	CGBasicGrassVertex *vertices = nullptr;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	inputLayout = nullptr;
	numBlades = 0;

	try
	{
		if (!device || !vsBytecode)
			throw("Invalid parameters for basic grass model instantiation");


		// Create array of points in system memory - randomise (x, z) and store y as T(x, z) where T() gives the height of the basic terrain model at (x, z)
		vertices = (CGBasicGrassVertex*)malloc(numPoints * sizeof(CGBasicGrassVertex));

		if (!vertices)
			throw("Cannot create basic grass point buffer");
	
		DWORD terrain_hw = (terrainWidth - 1) / 2;
		DWORD terrain_hh = (terrainHeight - 1) / 2;

		// Setup vertex positions
		float scale = 0.5f;
		CGBasicGrassVertex *vptr = vertices;

		// Generate 'noofVertices' points at random positions on the terrain

		for (DWORD i=0; i<numPoints; ++i, ++vptr) {

			float x_ = ((float)rand()/(float)RAND_MAX) * (float)terrainWidth;
			float z_ = ((float)rand()/(float)RAND_MAX) * (float)terrainHeight;

			float x = scale * (x_ - (float)terrain_hw);
			float z = scale * (z_ - (float)terrain_hh);

			// set position
			vptr->pos.x = x;
			vptr->pos.z = z;
			vptr->pos.y = 0.1f * (z * sinf(x) + x * cosf(z));

			// set colour
			vptr->colour = XMCOLOR(0.0f, 0.5f, 0.2f, 1.0f);

			// set weight between 0 and 1
			vptr->weight = ((float)rand()/(float)RAND_MAX);
		}


		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGBasicGrassVertex) * numPoints;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = vertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");

		// dispose of local buffer resources since no longer needed
		free(vertices);

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGBasicGrassVertex::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");

		numBlades = numPoints;
	}
	catch (char *err)
	{
		cout << "Basic grass model could not be instantiated due to:\n";
		cout << err << endl << endl;
		
		if (vertices)
			free(vertices);

		if (vertexBuffer)
			vertexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		inputLayout = nullptr;
	}
}


void CGBasicGrass::render(ID3D11DeviceContext *context) {

	// validate basic grass model before rendering
	if (!context || !vertexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set grass model vertex buffer for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGBasicGrassVertex)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Draw grass model
	context->Draw(numBlades, 0);
}
