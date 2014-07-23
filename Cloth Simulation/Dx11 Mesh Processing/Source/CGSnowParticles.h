
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <string>
#include "CGBaseModel.h"


class CGPipeline;
class CGRasteriserStage;
class CGOutputMergerStage;


// Implement particle system to run on the GPU in DirectX 11 using the GS and SO stages.


// Constant buffer (system memory) models used by the snow particle system

_DECLSPEC_ALIGN_16_ struct cameraPositionStruct {

	XMMATRIX					viewMatrix;
	
	cameraPositionStruct() {

		ZeroMemory(this, sizeof(cameraPositionStruct));
		viewMatrix = XMMatrixIdentity();
	}
};

_DECLSPEC_ALIGN_16_ struct cameraProjectionStruct {

	XMMATRIX					projMatrix;
	FLOAT						nearPlane;
	FLOAT						farPlane;
	FLOAT						_pz, _pw; // padding

	cameraProjectionStruct() {

		ZeroMemory(this, sizeof(cameraProjectionStruct));
		projMatrix = XMMatrixIdentity();
		nearPlane = 0.1f;
		farPlane = 1000.0f;
	}
};

_DECLSPEC_ALIGN_16_ struct snowSystemUpdateConstantsStruct {

	XMFLOAT4		gravity;

	FLOAT			resetAge;
	FLOAT			ageDelta;
	FLOAT			generatorAgeThreshold;
	FLOAT			particleDestroyAge;
	
	// Pseudo-random numbers (x, y, z) are used for initial velocity values while w is used to initialise the new particle age
	XMFLOAT4		prn;
	
	// Pesudo-random number in the range [0, 8) to determine the initial particle type (so snowflakes 'live' for different lengths of time).  Again, only a single new particle likely in every update cycle  (see above)
	UINT			randomFlakeType;
	UINT			_py, _pz, _pw;

	snowSystemUpdateConstantsStruct() {

		ZeroMemory(this, sizeof(snowSystemUpdateConstantsStruct));
		
		gravity = XMFLOAT4(0.0f, -0.000005f, 0.0f, 0.0f);
		resetAge = 0.0f;
		ageDelta = 0.0025f;
		generatorAgeThreshold = 0.01f; // age of generator before new particle created
		particleDestroyAge = 1000.0f;
	}
};


// Particle model
struct CGSnowParticle  {

	XMFLOAT3			pos; // positions of particles are always in world space
	XMFLOAT3			velocity;
	FLOAT				weight;
	FLOAT				age; // use age to calculate colour / texture etc. (for a generator, age = 0 means we create another particle)
	FLOAT				theta;
	FLOAT				angularVelocity;
	BOOL				isaGenerator;
	UINT				flakeType;

	static HRESULT createInputLayout(ID3D11Device *device, ID3DBlob *shaderBlob, ID3D11InputLayout **layout);
};


class CGSnowParticleSystem : public CGBaseModel {
	
	// Snow particle effect pipeline configurations
	CGPipeline*							updatePipeline;
	CGPipeline*							renderPipeline;

	// Double-buffer particle system
	ID3D11Buffer						*Pb1;
	ID3D11Buffer						*Pb2;

	// Input layout for snowflake particle
	ID3D11InputLayout					*particleInputLayout;

	// Snowflake texture array
	ID3D11ShaderResourceView			*snowFlakeTextureArraySRV;

	// cbuffer buffers in system memory
	cameraPositionStruct				*cameraPositionBuffer;
	cameraProjectionStruct				*cameraProjectionBuffer;
	snowSystemUpdateConstantsStruct		*snowSystemUpdateConstantsBuffer;

	// cbuffer interfaces
	ID3D11Buffer						*cameraPosition_cbuffer;
	ID3D11Buffer						*cameraProjection_cbuffer;
	ID3D11Buffer						*snowSystemUpdateConstants_cbuffer;

	// General particle system variables
	DWORD								numInitialParticles;
	DWORD								maxParticles;
	BOOL								firstRun;

	ID3D11Buffer						*sourceBuffer, *resultBuffer;

	

#pragma region Private interface

	void initialiseParticleBuffers(ID3D11Device *device, const float setupRadius, const DWORD particleBufferSize, const DWORD numInitialParticles);

#pragma endregion

public:

	CGSnowParticleSystem(ID3D11Device *device, ID3D11DeviceContext *context, DWORD _numInitialParticles, DWORD _maxParticles, float setupRadius, CGRasteriserStage *updateRS, CGOutputMergerStage *updateOM, CGRasteriserStage *renderRS, CGOutputMergerStage *renderOM);

	~CGSnowParticleSystem();
	
	void setCameraViewMatrix(CXMMATRIX _viewMatrix);
	void setCameraProjectionState(CXMMATRIX _projMatrix, const FLOAT _nearPlaneDist, const float _farPlaneDist);

	//void render(ID3D11DeviceContext *context) {};
	void render(ID3D11DeviceContext *context);
};
