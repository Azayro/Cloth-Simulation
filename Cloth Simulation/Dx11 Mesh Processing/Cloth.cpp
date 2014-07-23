#include "Cloth.h"
#include <iostream>
#include "Source\CGVertexExt.h"

using namespace std;
using namespace CoreStructures;

// Constructor
Cloth::Cloth(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD clothW, DWORD clothH)
{
	// Initialise variables
	vertexBuffer		= NULL;
	indexBuffer			= NULL;
	inputLayout			= NULL;
	constraintBuffer	= NULL;
	anchorBuffer		= NULL;

	w = clothW;
	h = clothH;

	totalConstraints = ((((w - 2) * 4) + 5) * (h - 1)) + (w - 1);

	clothForces			= nullptr;
	clothConstraints	= nullptr;
	clothAnchors		= nullptr;

	anchorOn			= true;

	// Call the buffer setup
	setupBuffers(device, vsBytecode);
	compileClothShaders(device);
}

// Destructor
Cloth::~Cloth()
{

}

// Buffer setup
void Cloth::setupBuffers(ID3D11Device *device, ID3DBlob *vsBytecode)
{
	// Setup basic terrain model buffers
	Particle* vertices			= nullptr;
	Constraint* constraints		= nullptr;
	Anchor* anchors				= nullptr;
	DWORD* indices				= nullptr;
	
	try
	{
		if (!device || !vsBytecode)
			throw("Invalid parameters for cloth model model instantiation");


		vertices = (Particle*)malloc(w * h * sizeof(Particle));
		indices = (DWORD*)malloc((w-1) * (h-1) * 6 * sizeof(DWORD));
		anchors = (Anchor*)malloc(sizeof(Anchor) * 3);
		constraints = (Constraint*)malloc(sizeof(Constraint) * totalConstraints);

		if (!vertices || !indices)
		{
			throw("Cannot create cloth buffers");
		}

#pragma region Vertices Setup
		// Setup vertices positions
		Particle *vptr = vertices;

		for (int j=0; j<int(h); ++j) 
		{
			for (int i = 0; i < int(w); ++i, ++vptr)
			{
				vptr->vertex.pos			= XMFLOAT3( ((float)i / (float)(w-1)), 0, ((float)j / (float)(h-1)));
				vptr->prevPos				= vptr->vertex.pos;
				vptr->vertex.normal			= XMFLOAT3(0, 0, 1);
				vptr->vertex.texCoord		= XMFLOAT2((float)i / (float)(w-1), (float)j/(float)(h-1));

				vptr->vertex.matDiffuse		= XMCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
				vptr->vertex.matSpecular	= XMCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
#pragma endregion

#pragma region Old Constraints Setup
		/*
		int constraintI = 0;
		int index = 0;
		GUVector3 distance;

		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				index = (j * w) + i;

				// Vertical
				if(j)
				{
					// Vertical structured constraint
					constraints[constraintI].start	= index - w;
					constraints[constraintI].end	= index;

					// Calculate distance
					distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-w].vertex.pos.x,
											vertices[index].vertex.pos.y - vertices[index-w].vertex.pos.y,
											vertices[index].vertex.pos.z - vertices[index-w].vertex.pos.z);

					// Set the constraints length to the calculated length
					constraints[constraintI].length = distance.length();

					constraintI++;

					if(i < (w -1))
					{
						// Up-right sheer constraint
						constraints[constraintI].start	= index - (w-1);
						constraints[constraintI].end	= index;

						// Calculate distance between the two points
						distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index -(w-1)].vertex.pos.x,
												vertices[index].vertex.pos.y - vertices[index -(w-1)].vertex.pos.y,
												vertices[index].vertex.pos.z - vertices[index -(w-1)].vertex.pos.z);

						// Set the constraints length to the calculated length
						constraints[constraintI].length = distance.length();

						// Increment constraint index
						constraintI++;
					}
				}

				// Horizontal
				if(i)
				{
					// Horizontal structured constraint
					constraints[constraintI].start	= index - 1;
					constraints[constraintI].end	= index;

					// Calculate distance
					distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-1].vertex.pos.x,
											vertices[index].vertex.pos.y - vertices[index-1].vertex.pos.y,
											vertices[index].vertex.pos.z - vertices[index-1].vertex.pos.z);

					// Set the constraints length to the calculated length
					constraints[constraintI].length = distance.length();

					//Increment constraint index
					constraintI++;

					if(j)
					{
						// Up-left sheer constraint
						constraints[constraintI].start	= index - (w+1);
						constraints[constraintI].end	= index;

						// Calculate distance
						distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-(w+1)].vertex.pos.x,
												vertices[index].vertex.pos.y - vertices[index-(w+1)].vertex.pos.y,
												vertices[index].vertex.pos.z - vertices[index-(w+1)].vertex.pos.z);

						// Set the constraints length to the calculated length
						constraints[constraintI].length = distance.length();

						//Increment constraint index
						constraintI++;
					}
				}
			}
		}
		*/

#pragma endregion

#pragma region New Batch Constraints Setup

		int constraintI = 0;
		int index = 0;
		GUVector3 distance;

		// Batch sizes setup
		batchSize[0] = h * (w * 0.5);						// Horizontal Even
		batchSize[1] = ((w - 1) * h) - batchSize[0];		// Horizontal Odd
		batchSize[2] = w * (h * 0.5);						// Vertical Even
		batchSize[3] = ((h - 1) * w) - batchSize[2];		// Vertical Odd
		batchSize[4] = (w - 1) * (h * 0.5);					// Diagonal Even
		batchSize[5] = ((w - 1) * (h - 1)) - batchSize[4];	// Diagonal Odd
		batchSize[6] = batchSize[4];
		batchSize[7] = batchSize[5];

		constraintBatch[0] = 0;
		constraintBatch[1] = batchSize[0];
		constraintBatch[2] = constraintBatch[1] + batchSize[1];
		constraintBatch[3] = constraintBatch[2] + batchSize[2];
		constraintBatch[4] = constraintBatch[3] + batchSize[3];
		constraintBatch[5] = constraintBatch[4] + batchSize[4];
		constraintBatch[6] = constraintBatch[5] + batchSize[5];
		constraintBatch[7] = constraintBatch[6] + batchSize[6];


		// Even and odd booleans
		bool oddHori = true;
		bool oddVert = true;

		for (int j = 0; j < h; j++)
		{
			// Vertical boolean flip
			oddVert = !oddVert;

			for (int i = 0; i < w; i++)
			{
				index = (j * w) + i;

				// Horizontal boolean flip
				oddHori = !oddHori;

				// Horizontal constraints
				if(i)
				{
					if(oddHori)
					{
						constraintI = constraintBatch[0];
						constraintBatch[0]++;
					}
					else
					{
						constraintI = constraintBatch[1];
						constraintBatch[1]++;
					}

					// Horizontal structured constraint
					constraints[constraintI].start	= index - 1;
					constraints[constraintI].end	= index;

					// Calculate distance
					distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-1].vertex.pos.x,
											vertices[index].vertex.pos.y - vertices[index-1].vertex.pos.y,
											vertices[index].vertex.pos.z - vertices[index-1].vertex.pos.z);

					// Set the constraints length to the calculated length
					constraints[constraintI].length = distance.length();

					// Up and left shear constraints
					if(j)
					{
						if(oddVert)
						{
							constraintI = constraintBatch[4];
							constraintBatch[4]++;
						}
						else
						{
							constraintI = constraintBatch[5];
							constraintBatch[5]++;
						}

						constraints[constraintI].start	= index - (w+1);
						constraints[constraintI].end	= index;

						// Calculate distance
						distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-(w+1)].vertex.pos.x,
												vertices[index].vertex.pos.y - vertices[index-(w+1)].vertex.pos.y,
												vertices[index].vertex.pos.z - vertices[index-(w+1)].vertex.pos.z);

						// Set the constraints length to the calculated length
						constraints[constraintI].length = distance.length();
					}
				}

				// Vertical constraints
				if(j)
				{
					if(oddVert)
					{
						constraintI = constraintBatch[2];
						constraintBatch[2]++;
					}
					else
					{
						constraintI = constraintBatch[3];
						constraintBatch[3]++;
					}

					// Vertical structured constraint
					constraints[constraintI].start	= index - w;
					constraints[constraintI].end	= index;

					// Calculate distance
					distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index-w].vertex.pos.x,
											vertices[index].vertex.pos.y - vertices[index-w].vertex.pos.y,
											vertices[index].vertex.pos.z - vertices[index-w].vertex.pos.z);

					// Set the constraints length to the calculated length
					constraints[constraintI].length = distance.length();

					// Up and right shear constraint
					if(i < (w -1))
					{
						if(oddVert)
						{
							constraintI = constraintBatch[6];
							constraintBatch[6]++;
						}
						else
						{
							constraintI = constraintBatch[7];
							constraintBatch[7]++;
						}

						// Up-right sheer constraint
						constraints[constraintI].start	= index - (w-1);
						constraints[constraintI].end	= index;

						// Calculate distance between the two points
						distance = GUVector3(	vertices[index].vertex.pos.x - vertices[index -(w-1)].vertex.pos.x,
												vertices[index].vertex.pos.y - vertices[index -(w-1)].vertex.pos.y,
												vertices[index].vertex.pos.z - vertices[index -(w-1)].vertex.pos.z);

						// Set the constraints length to the calculated length
						constraints[constraintI].length = distance.length();
					}
				}
			}
		}

#pragma endregion

#pragma region Anchor Setup
		// Anchors index setup
		anchors[0].index = 0;
		anchors[1].index = (DWORD)(w/2);
		anchors[2].index = w-1;

		// Anchors position setup
		for(int i = 0; i<3; i++)
			anchors[i].pos = vertices[anchors[i].index].vertex.pos;
#pragma endregion

#pragma region Indices Setup
		// Setup index values
		DWORD *iptr = indices;

		for (DWORD j=0; j<h-1; ++j) 
		{
			for (DWORD i=0; i<w-1; ++i, iptr+=6) 
			{
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
#pragma endregion

#pragma region BUFFERS

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		vertexDesc.CPUAccessFlags		= 0;
		vertexDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		vertexDesc.StructureByteStride	= sizeof(Particle);
		vertexDesc.Usage				= D3D11_USAGE_DEFAULT;
		vertexDesc.ByteWidth			= sizeof(Particle) * w * h;
		vertexData.pSysMem				= vertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw("Vertex buffer cannot be created");


		// Setup Constraint buffer
		D3D11_BUFFER_DESC constraintDesc;
		D3D11_SUBRESOURCE_DATA constraintData;

		ZeroMemory(&constraintDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&constraintData, sizeof(D3D11_SUBRESOURCE_DATA));

		constraintDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		constraintDesc.CPUAccessFlags		= 0;
		constraintDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		constraintDesc.StructureByteStride	= sizeof(Constraint);
		constraintDesc.Usage				= D3D11_USAGE_DEFAULT;
		constraintDesc.ByteWidth			= sizeof(Constraint) * totalConstraints;
		constraintData.pSysMem				= constraints;

		hr = device->CreateBuffer(&constraintDesc, &constraintData, &constraintBuffer);

		if (!SUCCEEDED(hr))
			throw("Constraint buffer cannot be created");

		// Setup Anchor buffer
		D3D11_BUFFER_DESC anchorDesc;
		D3D11_SUBRESOURCE_DATA anchorData;

		ZeroMemory(&anchorDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&anchorData, sizeof(D3D11_SUBRESOURCE_DATA));

		anchorDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		anchorDesc.CPUAccessFlags		= 0;
		anchorDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		anchorDesc.StructureByteStride	= sizeof(Anchor);
		anchorDesc.Usage				= D3D11_USAGE_DEFAULT;
		anchorDesc.ByteWidth			= sizeof(Anchor) * 3;
		anchorData.pSysMem				= anchors;

		hr = device->CreateBuffer(&anchorDesc, &anchorData, &anchorBuffer);

		if (!SUCCEEDED(hr))
			throw("Anchor buffer cannot be created");

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

		// build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = CGVertexExt::createInputLayout(device, vsBytecode, &inputLayout);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create input layout interface");

#pragma endregion

#pragma region Resource Views
		// Create UAV for particles buffer
		D3D11_UNORDERED_ACCESS_VIEW_DESC particlesUAVDesc;

		particlesUAVDesc.Buffer.FirstElement		= 0;
		particlesUAVDesc.Buffer.Flags				= 0;
		particlesUAVDesc.Buffer.NumElements			= w * h;
		particlesUAVDesc.Format						= DXGI_FORMAT_UNKNOWN;
		particlesUAVDesc.ViewDimension				= D3D11_UAV_DIMENSION_BUFFER;

		hr = device->CreateUnorderedAccessView(vertexBuffer, &particlesUAVDesc, &particlesUAV); 

		if (!SUCCEEDED(hr))
			throw("Cannot create particles UAV");

		/*
		// Create shader resource view for constraints
		D3D11_SHADER_RESOURCE_VIEW_DESC constraintSRVDesc;

		constraintSRVDesc.Buffer.FirstElement		= 0;
		constraintSRVDesc.Buffer.NumElements		= totalConstraints;
		constraintSRVDesc.Format					= DXGI_FORMAT_UNKNOWN;
		constraintSRVDesc.ViewDimension				= D3D11_SRV_DIMENSION_BUFFER;

		hr = device->CreateShaderResourceView(constraintBuffer, &constraintSRVDesc, &constraintSRV);
		*/

		// ------------------------------------------------------
		// Create Shader Resource Views for each batch constraint
		// ------------------------------------------------------
		int firstEl = 0;

		for (int i = 0; i < 8; i++)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC constraintSRVDesc;

			constraintSRVDesc.Buffer.FirstElement		= firstEl;
			constraintSRVDesc.Format					= DXGI_FORMAT_UNKNOWN;
			constraintSRVDesc.ViewDimension				= D3D11_SRV_DIMENSION_BUFFER;

			constraintSRVDesc.Buffer.NumElements	= batchSize[i];
			firstEl += batchSize[i];

			hr = device->CreateShaderResourceView(constraintBuffer, &constraintSRVDesc, &constraintBatchSRV[i]);

			if (!SUCCEEDED(hr))
				throw("Cannot create constraints SRV");
		}

		

		// Create shader resource view for anchors
		D3D11_SHADER_RESOURCE_VIEW_DESC anchorSRVDesc;

		anchorSRVDesc.Buffer.FirstElement			= 0;
		anchorSRVDesc.Buffer.NumElements			= 3;
		anchorSRVDesc.Format						= DXGI_FORMAT_UNKNOWN;
		anchorSRVDesc.ViewDimension					= D3D11_SRV_DIMENSION_BUFFER;

		hr = device->CreateShaderResourceView(anchorBuffer, &anchorSRVDesc, &anchorSRV);

		if (!SUCCEEDED(hr))
			throw("Cannot create anchors SRV");


#pragma endregion

		// dispose of local buffer resources since no longer needed
		free(vertices);
		free(indices);
		free(constraints);
		free(anchors);
	}
	catch (char *err)
	{
		cout << "Cloth could not be instantiated due to:\n";
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

		if (constraintBuffer)
			constraintBuffer->Release();

		if (anchorBuffer)
			anchorBuffer->Release();

		vertexBuffer		= nullptr;
		indexBuffer			= nullptr;
		inputLayout			= nullptr;
		constraintBuffer	= nullptr;
		anchorBuffer		= nullptr;

		w = 0;
		h = 0;
	}
}

// Compile and create shaders
void Cloth::compileClothShaders(ID3D11Device *device)
{
	// Create and compile shaders
	try
	{
		ID3DBlob* cShaderBlob = nullptr;

#pragma region Force Shader
		// Force shader compile and create

		// Compile shader
		HRESULT hr = CShaderFactory::CompileComputeShader(L"Resources\\Shaders\\cloth_forces_cs.hlsl", "main", device, &cShaderBlob);

		if(!SUCCEEDED(hr))
			throw("Forces shader compile error");

		// Create shader
		hr = device->CreateComputeShader(cShaderBlob->GetBufferPointer(), cShaderBlob->GetBufferSize(), nullptr, &clothForces);

		cShaderBlob->Release();

		if(!SUCCEEDED(hr))
			throw("Forces shader create error");

#pragma endregion

#pragma region Constraint Shader
		// Constraint shader compile and create
		
		// Compile shader
		hr = CShaderFactory::CompileComputeShader(L"Resources\\Shaders\\cloth_constraints_cs.hlsl", "main", device, &cShaderBlob);

		if(!SUCCEEDED(hr))
			throw("Constraints shader compile error");

		// Create shader
		hr = device->CreateComputeShader(cShaderBlob->GetBufferPointer(), cShaderBlob->GetBufferSize(), nullptr, &clothConstraints);

		cShaderBlob->Release();

		if(!SUCCEEDED(hr))
			throw("Constraints shader create error");

#pragma endregion

#pragma region Anchor Shader
		// Anchor shader compile and create

		// Compile shader
		hr = CShaderFactory::CompileComputeShader(L"Resources\\Shaders\\cloth_anchors_cs.hlsl", "main", device, &cShaderBlob);

		if(!SUCCEEDED(hr))
			throw("Anchors shader compile error");

		// Create shader
		hr = device->CreateComputeShader(cShaderBlob->GetBufferPointer(), cShaderBlob->GetBufferSize(), nullptr, &clothAnchors);

		cShaderBlob->Release();

		if(!SUCCEEDED(hr))
			throw("Anchors shader create error");

#pragma endregion

	}
	catch(char* err)
	{
		cout << err;
	}
}

// Update
void Cloth::update(ID3D11DeviceContext* context)
{
	// Bind Unordered Access View to the compute shader
	context->CSSetUnorderedAccessViews(0, 1, &particlesUAV, nullptr);
	
	// Apply forces shader
	context->CSSetShader(clothForces, 0, 0);
	context->Dispatch((int)(w*h), 1, 1);

	// Apply anchors shader
	if(anchorOn)
	{
		context->CSSetShader(clothAnchors, 0, 0);
		context->Dispatch(3, 1, 1);
	}

	// Apply constraints shader
	context->CSSetShader(clothConstraints, 0, 0);
	//context->Dispatch(totalConstraints, 1, 1);

	// Bind SRVs
	ID3D11ShaderResourceView* SRV[] = {constraintBatchSRV[0], anchorSRV};

	for(int i = 0; i < 8; i++)
	{
		SRV[0] = constraintBatchSRV[i];
		context->CSSetShaderResources(0, 2, SRV); 
		context->Dispatch(batchSize[i], 1, 1);
	}

	// Unbind UAV
	ID3D11UnorderedAccessView* noUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &noUAV, nullptr);
}

// Render cloth
void Cloth::render(ID3D11DeviceContext *context)
{
	// validate cloth model before rendering
	if (!context || !vertexBuffer || !indexBuffer || !inputLayout)
		return;

	// Update 
	update(context);

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set basic terrain model vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = {vertexBuffer};
	UINT vertexStrides[] = {sizeof(Particle)};
	UINT vertexOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw basic terrain model
	context->DrawIndexed((w-1) * (h-1) * 6, 0, 0);

}

