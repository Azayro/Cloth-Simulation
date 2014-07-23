
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <string>



class CGTextureLoader {

public:

	// Setup ID3D11Texture2D interfaces from images loaded using Windows Imaging Component (WIC)
	static HRESULT loadTexture(const std::wstring& textureFilePath, ID3D11Device *device, ID3D11Texture2D **texture);

	static HRESULT loadDDSTextureArray(ID3D11Device *device, ID3D11DeviceContext *context, const std::wstring* filenames, const DWORD numTextures, ID3D11ShaderResourceView** arraySRV);

};

