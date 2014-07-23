
#include "CGPipeline.h"
#include <iostream>
#include "HLSLFactory.h"
#include "CGRasteriserStage.h"
#include "CGOutputMergerStage.h"


using namespace std;


// configure basic pipeline with default settings and a given vertex and pixel shader
CGPipeline::CGPipeline(
	ID3D11Device *device,
	ID3D11VertexShader *vs,
	ID3D11GeometryShader *gs,
	ID3D11PixelShader *ps,
	CGRasteriserStage *_rsStage,
	CGOutputMergerStage *_omStage)
{
	ZeroMemory(this, sizeof(CGPipeline));

	// validate parameters - must provide at least a device and fixed stage configuration before the pipeline can be valid
	if (!device)
		return;

	// Add references to shader objects
	
	// Mandatory shader stage (vertex shader)
	vertexShader = vs;
	if (vertexShader)
		vertexShader->AddRef();

	// Optional shader stage - geometry shader
	geometryShader = gs;
	if (geometryShader)
		geometryShader->AddRef();

	// Mandatory shader stage (pixel shader)
	pixelShader = ps;
	if (pixelShader)
		pixelShader->AddRef();

	// Set Rasteriser (RS) stage
	RSStage = _rsStage;

	// Setup Output Merger stage
	OMStage = _omStage;
}


CGPipeline::CGPipeline(
	ID3D11Device *device,
	const char* vsPath,
	const char* gsPath,
	const char* psPath,
	CGStreamOutConfig *soConfig,
	CGRasteriserStage *_rsStage,
	CGOutputMergerStage *_omStage,
	ID3DBlob **vsBytecode)
{
	ZeroMemory(this, sizeof(CGPipeline));

	if (!device)
		return;

	ID3D11VertexShader *vs = nullptr;
	ID3D11GeometryShader *gs = nullptr;
	ID3D11PixelShader *ps = nullptr;
	ID3DBlob *shaderBytecode = nullptr;

	try
	{
		HRESULT hr;

		if (vsPath)
			hr = HLSLFactory::loadVertexShader(device, string(vsPath), &vs, &shaderBytecode);

		if (gsPath)
			hr = HLSLFactory::loadGeometryShader(device, string(gsPath), soConfig, &gs);

		if (psPath)
			hr = HLSLFactory::loadPixelShader(device, string(psPath), &ps);

		if (vs)
			vertexShader = vs;

		if (gs)
			geometryShader = gs;

		if (ps)
			pixelShader = ps;

		if (vsBytecode)
			*vsBytecode = shaderBytecode;
	}
	catch (char*)
	{
		cout << "Pipeline could not be setup due to shader compilation errors\n\n";

		// Cleanup
		if (vs)
			vs->Release();

		if (gs)
			gs->Release();

		if (ps)
			ps->Release();

		if (shaderBytecode)
			shaderBytecode->Release();
	}

	// Set Rasteriser (RS) stage
	RSStage = _rsStage;

	// Setup Output Merger stage
	OMStage = _omStage;
}


CGPipeline::~CGPipeline() {

	if (vertexShader)
		vertexShader->Release();

	if (geometryShader)
		geometryShader->Release();

	if (pixelShader)
		pixelShader->Release();
}


bool CGPipeline::applyPipeline(ID3D11DeviceContext *context) {

	// Bind shaders
	context->VSSetShader(vertexShader, 0, 0);
	context->HSSetShader(0, 0, 0);
	context->DSSetShader(0, 0, 0);
	context->GSSetShader(geometryShader, 0, 0);
	context->PSSetShader(pixelShader, 0, 0);
	context->CSSetShader(0, 0, 0);

	// Set Rasteriser (RS) state
	if (RSStage)
		RSStage->applyRSStage(context);

	// Set Output Merger (OM) state
	if (OMStage)
		OMStage->applyOMStage(context);

	return true;
}
