
#pragma once

#include <D3DX11.h>
#include <xnamath.h>


// templated function to map constant buffers or 1D resources that are created as dynamnic buffers with CPU write access
template <class T>
HRESULT mapBuffer(ID3D11DeviceContext *context, T *srcBuffer, ID3D11Buffer *buffer) {

	D3D11_MAPPED_SUBRESOURCE res;

	HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	if (SUCCEEDED(hr)) {

		memcpy(res.pData, srcBuffer, sizeof(T));
		context->Unmap(buffer, 0);
	}

	return hr;
}

template <class T>
HRESULT createCBuffer(ID3D11Device *device, T *srcBuffer, ID3D11Buffer **cBuffer) {

	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;

	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	cbufferDesc.ByteWidth = sizeof(T);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = srcBuffer;
		
	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, cBuffer);

	return hr;
}




_DECLSPEC_ALIGN_16_ struct cameraStruct {

	XMMATRIX					viewProjMatrix;
	
	XMFLOAT3					eyePos;
	FLOAT						_pad01;

	cameraStruct() {

		ZeroMemory(this, sizeof(cameraStruct));

		viewProjMatrix = XMMatrixIdentity();
		eyePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
};

_DECLSPEC_ALIGN_16_ struct worldTransformStruct {

	XMMATRIX					worldMatrix;
	XMMATRIX					normalMatrix;

	worldTransformStruct() {

		worldMatrix = XMMatrixIdentity();
		normalMatrix = XMMatrixIdentity();
	}
};

_DECLSPEC_ALIGN_16_ struct gameTimeStruct {

	FLOAT					gameTime;
	FLOAT					_py, _pz, _pw; // pad to float4 16 byte alignment

	gameTimeStruct() {

		ZeroMemory(this, sizeof(gameTimeStruct));
		gameTime = 0.0f;
	}
};


// Model a single point light source.  Padding is applied to align the memory footprint of the structure with the corresponding HLSL cbuffer structure
struct CGPointLightStruct {

	XMFLOAT3		pos;
	FLOAT			range;
	XMFLOAT4		ambient;
	XMFLOAT4		diffuse;
	XMFLOAT4		specular;
	XMFLOAT3		attenuation;
	FLOAT			_pad01; // pad structure to same size as HLSL cbuffer otherwise createbuffer will return invalid argument

	CGPointLightStruct() {

		ZeroMemory(this, sizeof(CGPointLightStruct));
	}

	CGPointLightStruct(const XMFLOAT3& initPos, const FLOAT initRange, const XMFLOAT4& initAmbient, const XMFLOAT4& initDiffuse, const XMFLOAT4& initSpecular, const XMFLOAT3& initAttenuation) {

		ZeroMemory(this, sizeof(CGPointLightStruct));

		pos = initPos;
		range = initRange;
		ambient = initAmbient;
		diffuse = initDiffuse;
		specular = initSpecular;
		attenuation = initAttenuation;
	}
};


_DECLSPEC_ALIGN_16_ struct lightModelStruct {

	CGPointLightStruct			pointLight;

	lightModelStruct() : pointLight() {}
};

