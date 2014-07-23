
#include "CGBasicTerrain.h"
#include <iostream>
#include "CGVertexExt.h"

using namespace std;


// terrain height kernel
static inline float T(float x, float z) {

	return 0.1f * (z * sinf(x) + x * cosf(z));
}

// return the x, z plane component based on the scale and spatial extent of the terrain
static inline float P(int i, float scale, DWORD tSize) {

	return scale * ((float)i - (float)tSize);
}


CGBasicTerrain::CGBasicTerrain(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD newTerrainWidth, DWORD newTerrainHeight) {

	// Setup basic terrain model buffers
	CGVertexExt* vertices = nullptr;
	DWORD* indices = nullptr;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	inputLayout = nullptr;
	w = 0;
	h = 0;

	try
	{
		if (!device || !vsBytecode)
			throw("Invalid parameters for basic terrain model model instantiation");

		// Setup vertex and index buffer arrays in system memory
		w = newTerrainWidth;
		h = newTerrainHeight;

		vertices = (CGVertexExt*)malloc(w * h * sizeof(CGVertexExt));
		indices = (DWORD*)malloc((w-1) * (h-1) * 6 * sizeof(DWORD));

		if (!vertices || !indices)
			throw("Cannot create basic terrain model buffers");

		DWORD terrain_hw = (w - 1) / 2;
		DWORD terrain_hh = (h - 1) / 2;

		// Setup vertex positions
		float scale = 0.5f;
		
		CGVertexExt *vptr = vertices;
		
		for (int j=0; j<int(h); ++j) {

			float z = P(j, scale, terrain_hh);
			float z1 = P(j-1, scale, terrain_hh);
			float z2 = P(j+1, scale, terrain_hh);

			for (int i=0; i<int(w); ++i, ++vptr) {

				float x = P(i, scale, terrain_hw);
				
				float x1 = P(i-1, scale, terrain_hw);
				float x2 = P(i+1, scale, terrain_hw);

				XMFLOAT3 _z1 = XMFLOAT3(x, T(x, z1), z1);
				XMFLOAT3 _z2 = XMFLOAT3(x, T(x, z2), z2);

				XMFLOAT3 _x1 = XMFLOAT3(x1, T(x1, z), z);
				XMFLOAT3 _x2 = XMFLOAT3(x2, T(x2, z), z);

				XMFLOAT4 dx = XMFLOAT4(_x2.x - _x1.x, _x2.y - _x1.y, _x2.z - _x1.z, 0.0f);
				XMFLOAT4 dz = XMFLOAT4(_z2.x - _z1.x, _z2.y - _z1.y, _z2.z - _z1.z, 0.0f);

				XMVECTOR _dx = XMLoadFloat4(&dx);
				XMVECTOR _dz = XMLoadFloat4(&dz);

				XMVECTOR N = XMVector3Cross(_dz, _dx);

				// set position
				vptr->pos.x = x;
				vptr->pos.z = z;
				vptr->pos.y = T(x, z);

				XMStoreFloat3(&(vptr->normal), N);

				float r = 1.0f;//(float)i / (float)(w-1);
				float r2 = 1.0f;//(float)j / (float)(h-1);

				vptr->matDiffuse = XMCOLOR(0.0f, r*r2, 0.0f, 1.0f);
				vptr->matSpecular = XMCOLOR(0.0, 0.0f, 0.0f, 0.0f);
				vptr->texCoord = XMFLOAT2((float)i / (float)(w-1), (float)j/(float)(h-1));
			}
		}

		// Setup index values
		DWORD *iptr = indices;
		for (DWORD j=0; j<h-1; ++j) {

			for (DWORD i=0; i<w-1; ++i, iptr+=6) {

				DWORD a = w * j + i;
				DWORD b = a + w;
				DWORD c = b + 1;
				DWORD d = a + 1;

				iptr[0] = a;
				iptr[1] = b;
				iptr[2] = d;

				iptr[3] = b;
				iptr[4] = c;
				iptr[5] = d;
			}
		}


		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(CGVertexExt) * w * h;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = vertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");


		// Setup index buffer
		D3D11_BUFFER_DESC indexDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));

		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(DWORD) * (w-1) * (h-1) * 6;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexData.pSysMem = indices;

		hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);

		if (!SUCCEEDED(hr))
			throw("Index buffer cannot be created");

		// dispose of local buffer resources since no longer needed
		free(vertices);
		free(indices);

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexExt::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");
	}
	catch (char *err)
	{
		cout << "Basic terrain model could not be instantiated due to:\n";
		cout << err << endl << endl;
		
		if (vertices)
			free(vertices);

		if (indices)
			free(indices);

		if (vertexBuffer)
			vertexBuffer->Release();

		if (indexBuffer)
			indexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		indexBuffer = nullptr;
		inputLayout = nullptr;
		w = 0;
		h = 0;
	}
}


void CGBasicTerrain::render(ID3D11DeviceContext *context) {

	// validate basic terrain model before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set basic terrain model vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(CGVertexExt)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw basic terrain model
	context->DrawIndexed((w-1) * (h-1) * 6, 0, 0);
}
