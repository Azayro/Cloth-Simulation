#pragma once

#include <D3DX11.h>
#include <xnamath.h>

#include "Source\CGBaseModel.h"
#include "Source\CGVertexExt.h"
#include "CoreStructures\CoreStructures.h"
#include "CShaderFactory.h"


// Structure for the particle
struct Particle
{
	CGVertexExt vertex;
	XMFLOAT3 prevPos;
};

struct Constraint
{
	// Start and end vertex of the constraint
	unsigned int start;
	unsigned int end;

	// Length of the constraint
	float length;
};

struct Anchor
{
	// Anchor index
	DWORD32 index;

	// Position of the anchor
	XMFLOAT3 pos;
};


class Cloth : public CGBaseModel
{
private:
	// Dimensions of the cloth
	DWORD		w, h;
	int totalConstraints;
	
	int batchSize[8];
	int constraintBatch[8];


	// Shader
	ID3D11ComputeShader* clothForces;
	ID3D11ComputeShader* clothConstraints;
	ID3D11ComputeShader* clothAnchors;

	// Unordered Access Views
	ID3D11UnorderedAccessView* particlesUAV;
	
	// Buffers
	ID3D11Buffer		*constraintBuffer;
	ID3D11Buffer		*anchorBuffer;

	// Shader Resource Views
	//ID3D11ShaderResourceView* constraintSRV;
	ID3D11ShaderResourceView* constraintBatchSRV[8];
	ID3D11ShaderResourceView* anchorSRV;


	// Buffer setup 
	void setupBuffers(ID3D11Device *device, ID3DBlob *vsBytecode);

	// Compile and create the shaders
	void compileClothShaders(ID3D11Device *device);

	// Update
	void update(ID3D11DeviceContext* context);

public:
	// Constructor
	Cloth(ID3D11Device *device, ID3DBlob *vsBytecode, DWORD clothW, DWORD clothH);
	// Destructor
	~Cloth();

	// Render the cloth
	void render (ID3D11DeviceContext *context);

	bool anchorOn;
};