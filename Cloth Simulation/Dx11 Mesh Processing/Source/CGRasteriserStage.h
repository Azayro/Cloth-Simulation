
#pragma once

#include <D3DX11.h>
#include <xnamath.h>


class CGRasteriserStage {

	D3D11_RASTERIZER_DESC			RSdesc;
	ID3D11RasterizerState			*RSstate;

	UINT							numViewports;
	D3D11_VIEWPORT					*viewportArray; // weak reference to viewport array (usually have a single viewport for application window)

	// add scissor rect configuration later
	
public:

	static void initialiseDefaultRasteriser(ID3D11Device *device, UINT vpArraySize, D3D11_VIEWPORT *vpArray);
	static void applyDefaultRasteriser(ID3D11DeviceContext *context);

	CGRasteriserStage(ID3D11Device *device, UINT vpArraySize, D3D11_VIEWPORT *vpArray);
	~CGRasteriserStage();

	void applyRSStage(ID3D11DeviceContext *context);

};
