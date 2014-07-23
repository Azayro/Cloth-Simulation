
#include "CGSnowParticles.h"
#include <iostream>
#include "CGTextureLoader.h"
#include "CGPipeline.h"
#include "CGOutputMergerStage.h"
#include "HLSLFactory.h"
#include "buffers.h"


using namespace std;


static const DWORD NUM_SNOWFLAKE_TYPES = 8;


#pragma region CGSnowParticle layout descriptor and interface setup

// Vertex input descriptor based on CGSnowParticle struct
static D3D11_INPUT_ELEMENT_DESC snowParticleVertexDesc[] = {

	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"WEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"ORIENTATION", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"ANGULARVELOCITY", 0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"GENERATOR", 0, DXGI_FORMAT_R32_UINT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"FLAKETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

HRESULT CGSnowParticle::createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout) {

	return device->CreateInputLayout(snowParticleVertexDesc, ARRAYSIZE(snowParticleVertexDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), layout);
}


// SO stream configuration for particle system.  Used to setup geometry shader object.  Fields in each array entry are: output stream index (from GS), semantic name, semantic index (useful if duplicate semantics used), start component, component count, index of SO output buffer slot to write to.
D3D11_SO_DECLARATION_ENTRY particleSODesc[] = {

    {0, "POSITION", 0, 0, 3, 0},
    {0, "VELOCITY", 0, 0, 3, 0},
    {0, "WEIGHT", 0, 0, 1, 0},
	{0, "AGE", 0, 0, 1, 0},
	{0, "ORIENTATION", 0, 0, 1, 0},
	{0, "ANGULARVELOCITY", 0, 0, 1, 0},
	{0, "GENERATOR", 0, 0, 1, 0},
	{0, "FLAKETYPE", 0, 0, 1, 0}
};

#pragma endregion



CGSnowParticleSystem::CGSnowParticleSystem(ID3D11Device *device, ID3D11DeviceContext *context, DWORD _numInitialParticles, DWORD _maxParticles, float setupRadius, CGRasteriserStage *updateRS, CGOutputMergerStage *updateOM, CGRasteriserStage *renderRS, CGOutputMergerStage *renderOM) {

	//
	// Setup pipeline objects
	//
	ID3DBlob* vsBytecode = nullptr;

	CGStreamOutConfig soConfig;
	UINT particleSOSizes[] = {sizeof(CGSnowParticle)};

	soConfig.streamOutSize = 8;
	soConfig.streamOutDeclaration = particleSODesc;
	soConfig.numVertexStrides = 1;
	soConfig.streamOutVertexStrides = particleSOSizes;
	soConfig.rasteriseStreamIndex = D3D11_SO_NO_RASTERIZED_STREAM;

	updatePipeline = new CGPipeline(device, "Resources\\Shaders\\snow_vs.hlsl", "Resources\\Shaders\\snow_update_gs.hlsl", nullptr, &soConfig, updateRS, updateOM, &vsBytecode);

	renderPipeline = new CGPipeline(device, "Resources\\Shaders\\snow_vs.hlsl", "Resources\\Shaders\\snow_render_gs.hlsl", "Resources\\Shaders\\snow_ps.hlsl", nullptr, renderRS, renderOM, nullptr);


	//
	// Initialise particle buffers
	//
	initialiseParticleBuffers(device, setupRadius, _maxParticles, _numInitialParticles);


	//
	// Create vertex / particle layout object
	//
	HRESULT hr = CGSnowParticle::createInputLayout(device, vsBytecode, &particleInputLayout);


	//
	// Setup texture array for snowflakes
	//

	// Define source texture filenames
	static const wstring filenames[8] = {

		L"Resources\\Textures\\snowflake1.dds",
		L"Resources\\Textures\\snowflake2.dds",
		L"Resources\\Textures\\snowflake3.dds",
		L"Resources\\Textures\\snowflake4.dds",
		L"Resources\\Textures\\snowflake5.dds",
		L"Resources\\Textures\\snowflake6.dds",
		L"Resources\\Textures\\snowflake7.dds",
		L"Resources\\Textures\\snowflake8.dds"
	};
	
	hr = CGTextureLoader::loadDDSTextureArray(device, context, filenames, NUM_SNOWFLAKE_TYPES, &snowFlakeTextureArraySRV);


	//
	// Setup constant buffers
	//

	// Create buffers in system memory to hold per-frame data to be passed to cbuffers
	cameraPositionBuffer = (cameraPositionStruct*)_aligned_malloc(sizeof(cameraPositionStruct), 16);
	new (cameraPositionBuffer)cameraPositionStruct();

	cameraProjectionBuffer = (cameraProjectionStruct*)_aligned_malloc(sizeof(cameraProjectionStruct), 16);
	new (cameraProjectionBuffer)cameraProjectionStruct();

	snowSystemUpdateConstantsBuffer = (snowSystemUpdateConstantsStruct*)_aligned_malloc(sizeof(snowSystemUpdateConstantsStruct), 16);
	new (snowSystemUpdateConstantsBuffer)snowSystemUpdateConstantsStruct();

	// Setup cbuffer objects
	hr = createCBuffer(device, cameraPositionBuffer, &cameraPosition_cbuffer);
	hr = createCBuffer(device, cameraProjectionBuffer, &cameraProjection_cbuffer);
	hr = createCBuffer(device, snowSystemUpdateConstantsBuffer, &snowSystemUpdateConstants_cbuffer);


	//
	// Setup general particle system state
	//

	numInitialParticles = _numInitialParticles;
	maxParticles = _maxParticles;
	firstRun = true;
	sourceBuffer = Pb1;
	resultBuffer = Pb2;
}


CGSnowParticleSystem::~CGSnowParticleSystem() {
}


void CGSnowParticleSystem::setCameraViewMatrix(CXMMATRIX _viewMatrix) {

	cameraPositionBuffer->viewMatrix = _viewMatrix;
}


void CGSnowParticleSystem::setCameraProjectionState(CXMMATRIX _projMatrix, const FLOAT _nearPlaneDist, const float _farPlaneDist) {

	cameraProjectionBuffer->projMatrix = _projMatrix;
	cameraProjectionBuffer->nearPlane = _nearPlaneDist;
	cameraProjectionBuffer->farPlane = _farPlaneDist;
}


void CGSnowParticleSystem::render(ID3D11DeviceContext *context) {

	// Set random number values in snowSystemUpdateConstantsBuffer
	const float rndrcp = 1.0f / (float)RAND_MAX;
	const float xScale = 1.5f;
	const float zScale = 1.5f;

	snowSystemUpdateConstantsBuffer->prn.x = ((float)rand() * rndrcp * 2.0f - 1.0f) * xScale;
	snowSystemUpdateConstantsBuffer->prn.y = (float)rand() * rndrcp * 2.0f - 1.0f;
	snowSystemUpdateConstantsBuffer->prn.z = ((float)rand() * rndrcp * 2.0f - 1.0f) * zScale;
	snowSystemUpdateConstantsBuffer->prn.w = (float)rand() * rndrcp * 200.0f; // initial age
	snowSystemUpdateConstantsBuffer->randomFlakeType = rand() % NUM_SNOWFLAKE_TYPES;


	// Map cbuffer data to GPU-side cbuffers
	mapBuffer<cameraPositionStruct>(context, cameraPositionBuffer, cameraPosition_cbuffer);
	mapBuffer<cameraProjectionStruct>(context, cameraProjectionBuffer, cameraProjection_cbuffer);
	mapBuffer<snowSystemUpdateConstantsStruct>(context, snowSystemUpdateConstantsBuffer, snowSystemUpdateConstants_cbuffer);


	// Declare buffer binding properties for the snow particle model
	UINT vertexStrides[] = {sizeof(CGSnowParticle)};
	UINT vertexOffsets[] = {0};


	//
	// Update pass
	//

	// Bind source buffer to IA and set point topology
	context->IASetVertexBuffers(0, 1, &sourceBuffer, vertexStrides, vertexOffsets);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetInputLayout(particleInputLayout);

	// Bind result buffer to SO
	context->SOSetTargets(1, &resultBuffer, vertexOffsets);
	
	// Bind cbuffers
	ID3D11Buffer* gsUpdatePhaseCBuffers[] = {snowSystemUpdateConstants_cbuffer};
	context->GSSetConstantBuffers(5, 1, gsUpdatePhaseCBuffers);

	// Bind update pipeline
	updatePipeline->applyPipeline(context);

	// Draw
	if (firstRun) {

		context->Draw(numInitialParticles, 0);
		firstRun = false;

	} else {

		context->DrawAuto();
	}

	// Unbind buffers from IA and SO since DX will not allow a resource to bind to a write / read binding point simultaneously

	ID3D11Buffer* nullbuffer[] = {0};
	UINT nullStrides[] = {0};
	UINT nullOffsets[] = {0};

	context->IASetVertexBuffers(0, 1, nullbuffer, nullStrides, nullOffsets);
	context->SOSetTargets(1, nullbuffer, nullOffsets);



	//
	// Render pass
	//

	// Bind result buffer generated above to IA
	context->IASetVertexBuffers(0, 1, &resultBuffer, vertexStrides, vertexOffsets);

	// Bind cbuffers (this leaves snowSystemUpdateConstants bound to b5 as set above)
	ID3D11Buffer* gsRenderPhaseCBuffers[] = {cameraPosition_cbuffer, 0, cameraProjection_cbuffer};
	context->GSSetConstantBuffers(0, 3, gsRenderPhaseCBuffers);

	// Bind texture resources
	ID3D11ShaderResourceView* snowflakeSRV[] = {snowFlakeTextureArraySRV};
	context->PSSetShaderResources(1, 1, snowflakeSRV);

	// Bind update pipeline
	renderPipeline->applyPipeline(context);

	// Draw
	context->DrawAuto();

	// Unbind buffers from IA ready for next iteration of render()
	context->IASetVertexBuffers(0, 1, nullbuffer, nullStrides, nullOffsets);


	//
	// Swap particle buffers for next iteration of the render() function
	//
	std::swap<ID3D11Buffer*>(sourceBuffer, resultBuffer);
}


#pragma region Private interface implementation

//
// Private methods
//

void CGSnowParticleSystem::initialiseParticleBuffers(ID3D11Device *device, const float setupRadius, const DWORD particleBufferSize, const DWORD numInitialParticles) {

	// Create vertex buffer in system memory to store points
	CGSnowParticle* initParticles = (CGSnowParticle*)calloc(particleBufferSize, sizeof(CGSnowParticle));
	
	// Setup initial (generator) positions
	CGSnowParticle* vptr = initParticles;

	for (DWORD i=0; i<numInitialParticles; ++i, ++vptr) {

		// Initialise position
		vptr->pos.x = ((float)rand()/(float)RAND_MAX) * setupRadius;
		vptr->pos.y = 24.0f;
		vptr->pos.z = ((float)rand()/(float)RAND_MAX) * setupRadius;

		// Initialise velocity
		vptr->velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		// Initialise weight (since initial particles are all generators we will not assign a weight - can do if needed)
		vptr->weight = 0.0f;

		// Initialise age
		vptr->age = 0.0f;

		// Initialise orientation (this is not relevant for generator particles)
		vptr->theta = 0.0f;
		vptr->angularVelocity = 0.0f;

		// Initialise generator flag
		vptr->isaGenerator = 1;

		// Initialise flake type - generators will not be rendered however
		vptr->flakeType = 0;
	}

	D3D11_BUFFER_DESC vertexDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

	// Tell DX11 buffer can be bound as input to the IA stage and output from the SO stage and we want GPU read/write access (DEFAULT usage)
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	vertexDesc.ByteWidth = sizeof(CGSnowParticle) * particleBufferSize;
	vertexData.pSysMem = initParticles;
	
	// Setup first buffer with initial particle set
	HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &Pb1);
	
	// Setup second buffer to be empty - just allocate space
	hr = device->CreateBuffer(&vertexDesc, NULL, &Pb2);
}

#pragma endregion
