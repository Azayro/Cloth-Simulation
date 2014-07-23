
#include "CGRasteriserStage.h"
#include <iostream>

using namespace std;

static CGRasteriserStage* defaultRasteriserStage = nullptr;


void CGRasteriserStage::initialiseDefaultRasteriser(ID3D11Device *device, UINT vpArraySize, D3D11_VIEWPORT *vpArray) {

	// Only allow initialisation once
	if (defaultRasteriserStage)
		return;

	defaultRasteriserStage = new CGRasteriserStage(device, vpArraySize, vpArray);
}


void CGRasteriserStage::applyDefaultRasteriser(ID3D11DeviceContext *context) {

	if (defaultRasteriserStage)
		defaultRasteriserStage->applyRSStage(context);
}



CGRasteriserStage::CGRasteriserStage(ID3D11Device *device, UINT vpArraySize, D3D11_VIEWPORT *vpArray) {

	ZeroMemory(this, sizeof(CGRasteriserStage));

	try
	{
		// Setup default rasteriser state
		RSdesc.FillMode = D3D11_FILL_SOLID;
		RSdesc.CullMode = D3D11_CULL_NONE;
		RSdesc.FrontCounterClockwise = false;
		RSdesc.DepthBias = 0;
		RSdesc.SlopeScaledDepthBias = 0.0f;
		RSdesc.DepthBiasClamp = 0.0f;
		RSdesc.DepthClipEnable = true;
		RSdesc.ScissorEnable = false;
		RSdesc.MultisampleEnable = false;
		RSdesc.AntialiasedLineEnable = false;

		if (vpArraySize==0 || vpArray==nullptr)
			throw("Cannot create Rasteriser stage object - Invalid viewport configuration");

		HRESULT hr = device->CreateRasterizerState(&RSdesc, &RSstate);

		if (!SUCCEEDED(hr))
			throw("Cannot create Rasteriser state interface");

		viewportArray = (D3D11_VIEWPORT*)malloc(vpArraySize * sizeof(D3D11_VIEWPORT));

		if (!viewportArray)
			throw("Cannot create Rasteriser stage object - Unable to create viewport array");

		memcpy_s(viewportArray, vpArraySize * sizeof(D3D11_VIEWPORT), vpArray, vpArraySize * sizeof(D3D11_VIEWPORT));
		numViewports = vpArraySize;
	}
	catch(char *err)
	{
		cout << err << endl;

		// cleanup
		if (RSstate)
			RSstate->Release();

		if (viewportArray)
			free(viewportArray);

		RSstate = nullptr;
		viewportArray = nullptr;
	}
}


CGRasteriserStage::~CGRasteriserStage() {

	if (RSstate)
		RSstate->Release();

	if (viewportArray)
		free(viewportArray);
}


void CGRasteriserStage::applyRSStage(ID3D11DeviceContext *context) {

	// Set rasteriser (RS) state - if RSstate was not initialised properly then RSstate remains a nullptr and this sets the default rasteriser properties
	context->RSSetState(RSstate);
	context->RSSetViewports(numViewports, viewportArray);
}

