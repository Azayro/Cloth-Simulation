
#include "CGCube.h"
#include <iostream>
#include "CGVertexExt.h"

using namespace std;


#pragma region Cube model data

static CGVertexExt cubeVertices[] = {
	
	{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.577f, -0.577f, -0.577f), XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(0.0f, 1.0f)},
	{XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT3(-0.577f, 0.577f, -0.577f), XMCOLOR(1.0f, 0.96f, 0.62f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT3(0.577f, 0.577f, -0.577f), XMCOLOR(1.0f, 0.0f, 0.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(1.0f, 0.0f)},
	{XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT3(0.577f, -0.577f, -0.577f), XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(1.0f, 1.0f)},
	{XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT3(-0.577f, -0.577f, 0.577f), XMCOLOR(0.0f, 0.0f, 1.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(1.0f, 1.0f)},
	{XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT3(-0.577f, 0.577f, 0.577f), XMCOLOR(1.0f, 1.0f, 0.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(1.0f, 0.0f)},
	{XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT3(0.577f, 0.577f, 0.577f), XMCOLOR(0.0f, 1.0f, 1.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(0.0f, 0.0f)},
	{XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT3(0.577f, -0.577f, 0.577f), XMCOLOR(1.0f, 0.0f, 1.0f, 1.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT2(0.0f, 1.0f)}};

static DWORD cubeIndices[] = {
	0, 1, 2,
	0, 2, 3,
	4, 6, 5,
	4, 7, 6,
	4, 5, 1,
	4, 1, 0,
	3, 2, 6,
	3, 6, 7, 
	1, 5, 6,
	1, 6, 2,
	4, 0, 3,
	4, 3, 7};

#pragma endregion


CGCube::CGCube(ID3D11Device *device, ID3DBlob *vsBytecode, ID3D11ShaderResourceView *tex_view, ID3D11SamplerState *_sampler) {

	// Setup cube buffers
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	inputLayout = nullptr;
	textureResourceView = nullptr;
	sampler = nullptr;

	try
	{
		if (!device || !vsBytecode)
			throw("Invalid parameters for cube model instantiation");

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGVertexExt) * 8;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = cubeVertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");


		// Setup index buffer
		D3D11_BUFFER_DESC indexDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));

		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(DWORD) * 12 * 3; // 12 faces, 3 indices per face
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexData.pSysMem = cubeIndices;

		hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);

		if (!SUCCEEDED(hr))
			throw("Index buffer cannot be created");

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexExt::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");

		textureResourceView = tex_view;
		sampler = _sampler;

		if (textureResourceView)
			textureResourceView->AddRef();

		if (sampler)
			sampler->AddRef();
	}
	catch (char *err)
	{
		cout << "Cube could not be instantiated due to:\n";
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


CGCube::~CGCube() {

	if (textureResourceView)
		textureResourceView->Release();

	if (sampler)
		sampler->Release();
}


void CGCube::render(ID3D11DeviceContext *context) {

	// validate cube before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set cube vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGVertexExt)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline
	context->PSSetShaderResources(0, 1, &textureResourceView);
	context->PSSetSamplers(0, 1, &sampler);

	// Draw cube
	context->DrawIndexed(36, 0, 0);
}
