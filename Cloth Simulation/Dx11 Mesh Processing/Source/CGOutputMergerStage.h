
#pragma once

#include <D3DX11.h>
#include <xnamath.h>


class CGOutputMergerStage {

	D3D11_BLEND_DESC			blendDesc;
	ID3D11BlendState			*blendState;

	D3D11_DEPTH_STENCIL_DESC	dsDesc;
	ID3D11DepthStencilState		*dsState;

	UINT						numRenderTargets;
	ID3D11RenderTargetView		**renderTargetViews;

	ID3D11DepthStencilView		*depthStencilView;
	
	// note: Add UAVs later

public:

	// Default constructor - setup a default OM model.  Store number of render targets, an array of render target views and a single depth-stencil view
	CGOutputMergerStage(ID3D11Device *device, UINT numRT, ID3D11RenderTargetView **_rtViews, ID3D11DepthStencilView *_dsView, D3D11_DEPTH_STENCIL_DESC *_dsDesc, D3D11_BLEND_DESC *_blendDesc);
	~CGOutputMergerStage();

	void applyOMStage(ID3D11DeviceContext *context);
};
