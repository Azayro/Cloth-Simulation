
#include "CGOutputMergerStage.h"


CGOutputMergerStage::CGOutputMergerStage(
	ID3D11Device *device,
	// Render targets
	UINT numRT,
	ID3D11RenderTargetView **_rtViews,
	ID3D11DepthStencilView *_dsView,
	// Stage configuration descriptors
	D3D11_DEPTH_STENCIL_DESC *_dsDesc,
	D3D11_BLEND_DESC *_blendDesc) {

	//
	// Setup depth-stencil state
	//

	if (_dsDesc) {

		memcpy_s(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC), _dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	} else {

		// Setup default depth-stencil descriptor
	
		ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	}

	//
	// Setup blend state
	//

	if (_blendDesc) {

		memcpy_s(&blendDesc, sizeof(D3D11_BLEND_DESC), _blendDesc, sizeof(D3D11_BLEND_DESC));

	} else {

		// Setup default blend state descriptor (no blend)
	
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

		blendDesc.AlphaToCoverageEnable = FALSE; // Use pixel coverage info from rasteriser (default)
		blendDesc.IndependentBlendEnable = FALSE; // The following array of render target blend properties uses the blend properties from RenderTarget[0] for ALL render targets

		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	// Create OM state interfaces
	HRESULT hr = device->CreateDepthStencilState(&dsDesc, &dsState);
	hr = device->CreateBlendState(&blendDesc, &blendState);

	// Store references to render target views
	numRenderTargets = numRT;
	renderTargetViews = _rtViews;
	depthStencilView = _dsView;
}


CGOutputMergerStage::~CGOutputMergerStage() {

	if (dsState)
		dsState->Release();

	if (blendState)
		blendState->Release();
}


void CGOutputMergerStage::applyOMStage(ID3D11DeviceContext *context) {

	context->OMSetDepthStencilState(dsState, 0);

	// Setup blend state with default modulation array for RGBA and default sample coverage mask
	context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);

	// "Plug-in" render targets
	context->OMSetRenderTargets(1, renderTargetViews, depthStencilView);
}
