
#include "CGTextureLoader.h"
#include <wincodec.h>
#include <iostream>

using namespace std;

static IWICImagingFactory			*wicFactory = nullptr;


HRESULT getWICFormatConverter(IWICFormatConverter **formatConverter) {
	
	if (!formatConverter || !wicFactory)
		return E_FAIL;
	else
		return wicFactory->CreateFormatConverter(formatConverter);
}


// Load and return an IWICBitmap interface representing the image loaded from path.
HRESULT loadWICBitmap(LPCWSTR path, IWICBitmap **bitmap) {

	if (!bitmap || !wicFactory)
		return E_FAIL;

	IWICBitmapDecoder *bitmapDecoder = NULL;
	IWICBitmapFrameDecode *imageFrame = NULL;
	IWICFormatConverter *formatConverter = NULL;
	
	*bitmap = NULL;

	// create image decoder
	HRESULT hr = wicFactory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);
	
	// validate number of frames
	UINT numFrames = 0;

	if (SUCCEEDED(hr)) {

		hr = bitmapDecoder->GetFrameCount(&numFrames);
	}

	if (SUCCEEDED(hr) && numFrames>0) {

		// decode the first image frame only (default)
		hr = bitmapDecoder->GetFrame(0, &imageFrame);
	}

	if (SUCCEEDED(hr)) {

		hr = wicFactory->CreateFormatConverter(&formatConverter);
	}

	WICPixelFormatGUID pixelFormat;

	if (SUCCEEDED(hr)) {

		// check we can convert to the required format GUID_WICPixelFormat32bppPBGRA			
		hr = imageFrame->GetPixelFormat(&pixelFormat);
	}

	BOOL canConvert = FALSE;

	if (SUCCEEDED(hr)) {

		hr = formatConverter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppPBGRA, &canConvert);
	}

	if (SUCCEEDED(hr) && canConvert==TRUE) {
		
		hr = formatConverter->Initialize(
		imageFrame,						// Input source to convert
		GUID_WICPixelFormat32bppPBGRA,	// Destination pixel format
		WICBitmapDitherTypeNone,		// Specified dither pattern
		NULL,							// Specify a particular palette 
		0.f,							// Alpha threshold
		WICBitmapPaletteTypeCustom		// Palette translation type
		);
	}

	if (SUCCEEDED(hr)) {

		// convert and create bitmap from converter
		hr = wicFactory->CreateBitmapFromSource(formatConverter, WICBitmapCacheOnDemand, bitmap);
	}


	// cleanup
	if (formatConverter)
		formatConverter->Release();

	if (imageFrame)
		imageFrame->Release();

	if (bitmapDecoder)
		bitmapDecoder->Release();

	// return result
	return hr;
}




//
// public interface
//

HRESULT CGTextureLoader::loadTexture(const std::wstring& textureFilePath, ID3D11Device *device, ID3D11Texture2D **texture) {

	// create WIC factory if not already created
	if (wicFactory==nullptr) {

		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

		if (!SUCCEEDED(hr)) {

			return E_FAIL;
		}
	}

	if (!device || !texture) {

		return E_FAIL;
	}

	IWICBitmap *textureBitmap = NULL;
	IWICBitmapLock *lock = NULL;
	
	HRESULT hr = loadWICBitmap(textureFilePath.c_str(), &textureBitmap);

	UINT w = 0, h = 0;

	if (SUCCEEDED(hr)) {

		hr = textureBitmap->GetSize(&w, &h);
	}

	WICRect rect = {0, 0, w, h};

	if (SUCCEEDED(hr)) {

		hr = textureBitmap->Lock(&rect, WICBitmapLockRead, &lock);
	}

	UINT bufferSize = 0;
	BYTE *buffer = NULL;

	if (SUCCEEDED(hr)) {

		hr = lock->GetDataPointer(&bufferSize, &buffer);
	}

	if (SUCCEEDED(hr)) {
		
		D3D11_TEXTURE2D_DESC desc;

		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1; // single mip level in the loaded image
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		// create subresource description to initialise the texture data
		D3D11_SUBRESOURCE_DATA initData;

		initData.pSysMem = (void*)buffer;
		initData.SysMemPitch = w << 2;
		initData.SysMemSlicePitch = 0; // Not needed since 2D image only!

		// create the texture interface
		hr = device->CreateTexture2D(&desc, &initData, texture);
	}


	// Clean-up local resources
	if (lock)
		lock->Release();

	if (textureBitmap)
		textureBitmap->Release();

	return hr;
}



// Note: This method uses D3DX (which has been depricated under Windows 8).  Update this later!
HRESULT CGTextureLoader::loadDDSTextureArray(ID3D11Device *device, ID3D11DeviceContext *context, const wstring* filenames, const DWORD numTextures, ID3D11ShaderResourceView** arraySRV) {

	HRESULT hr = S_OK;
	ID3D11Texture2D** sourceTextures = nullptr;
	ID3D11Texture2D* textureArray = nullptr;
	ID3D11ShaderResourceView *textureArrayView = nullptr;

	try
	{
		// Create source textures array (individual textures loaded from disk)
		sourceTextures = (ID3D11Texture2D**)calloc(numTextures, sizeof(ID3D11Texture2D*));

		if (!sourceTextures)
			throw("Cannot create source texture array");

		// Load individual source textures
		for (UINT i=0; i<numTextures; ++i) {

			D3DX11_IMAGE_LOAD_INFO		loadInfo;

			loadInfo.Width = D3DX11_FROM_FILE;
			loadInfo.Height = D3DX11_FROM_FILE;
			loadInfo.FirstMipLevel = 0;
			loadInfo.MipLevels = D3DX11_FROM_FILE;
			loadInfo.Usage = D3D11_USAGE_STAGING;
			loadInfo.BindFlags = 0; // We're not going to use these on the GPU - simply used for creating the texture array
			loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
			loadInfo.MiscFlags = 0;
			loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			loadInfo.Filter = D3DX11_FILTER_NONE;
			loadInfo.MipFilter = D3DX11_FILTER_NONE;
			loadInfo.pSrcInfo = 0;

			hr = D3DX11CreateTextureFromFile(device, filenames[i].c_str(), &loadInfo, 0, (ID3D11Resource**)(&sourceTextures[i]), 0);

			if (!SUCCEEDED(hr))
				throw("Cannot create texture array slice");
		}

		// Setup relevant data for the texture array

		// First obtain the texture DESC for one of the loaded textures above.  One DESC structure can be used for all source textures loaded above since it is assumed they all have the same format
		D3D11_TEXTURE2D_DESC		sourceTextureDesc;
	
		// Populate the source texture DESC with info from first loaded texture
		sourceTextures[0]->GetDesc(&sourceTextureDesc);

		// Create the texture array interface based on the loaded texture properties
		D3D11_TEXTURE2D_DESC		textureArrayDesc;
		
		ZeroMemory(&textureArrayDesc, sizeof(D3D11_TEXTURE2D_DESC));

		textureArrayDesc.Width = sourceTextureDesc.Width;
		textureArrayDesc.Height = sourceTextureDesc.Height;
		textureArrayDesc.MipLevels = sourceTextureDesc.MipLevels;
		textureArrayDesc.ArraySize = numTextures;
		textureArrayDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureArrayDesc.SampleDesc.Count = 1;
		textureArrayDesc.SampleDesc.Quality = 0;
		textureArrayDesc.Usage = D3D11_USAGE_DEFAULT; // array will be optimised for GPU use
		textureArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // used in shaders - not a render target
		textureArrayDesc.CPUAccessFlags = 0;
		textureArrayDesc.MiscFlags = 0;

		// Create the texture array
		hr = device->CreateTexture2D(&textureArrayDesc, 0, &textureArray);

		if (!SUCCEEDED(hr))
			throw("Cannot create texture array interface");

		// Copy source texture data into the array subresources.  Texture data is organised by texture then mip level so loop through textures and for each texture loop through mip levels.  Each mip-level image is a sub-resource of a given texture slice in the texture array.
		for (UINT i=0; i<numTextures; ++i) { // i is the texture slice index

			for (UINT j=0; j<sourceTextureDesc.MipLevels; ++j) { // j is the mip-level index
			
				D3D11_MAPPED_SUBRESOURCE mappedTexture;
				hr = context->Map(sourceTextures[i], j, D3D11_MAP_READ, 0, &mappedTexture);

				if (!SUCCEEDED(hr))
					throw("Cannot map subresource to create texture array content");

				UINT arraySubresourceIndex = D3D11CalcSubresource(j, i, sourceTextureDesc.MipLevels);
				context->UpdateSubresource(textureArray, arraySubresourceIndex, nullptr, mappedTexture.pData, mappedTexture.RowPitch, 0);

				context->Unmap(sourceTextures[i], j);
			}
		}

		// Create resource view for texture array
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

		viewDesc.Format = textureArrayDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		viewDesc.Texture2DArray.MostDetailedMip = 0;
		viewDesc.Texture2DArray.MipLevels = textureArrayDesc.MipLevels;
		viewDesc.Texture2DArray.FirstArraySlice = 0;
		viewDesc.Texture2DArray.ArraySize = numTextures;

		hr = device->CreateShaderResourceView(textureArray, &viewDesc, &textureArrayView);

		if (!SUCCEEDED(hr))
			throw("Cannot create Shader Resource View to the texture array");


		// Cleanup

		// Release individual source textures
		for (UINT i=0; i<numTextures; ++i) {

			if (sourceTextures[i])
				sourceTextures[i]->Release();
		}

		free(sourceTextures);

		// Release texture array object
		if (textureArray)
			textureArray->Release();


		// Set result interfaces
		
		if (arraySRV)
			*arraySRV = textureArrayView;
	}
	catch(char *err)
	{
		// Report exception
		cout << err << endl;

		// Cleanup

		// Release individual source textures
		if (sourceTextures) {

			for (UINT i=0; i<numTextures; ++i) {

				if (sourceTextures[i])
					sourceTextures[i]->Release();
			}

			free(sourceTextures);
		}

		// Release texture array object
		if (textureArray)
			textureArray->Release();

		// Release shader resource view
		if (textureArrayView)
			textureArrayView->Release();
	}

	return hr;
}
