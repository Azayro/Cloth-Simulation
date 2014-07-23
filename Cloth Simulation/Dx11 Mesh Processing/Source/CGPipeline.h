
#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <string>

class CGRasteriserStage;
class CGOutputMergerStage;
struct CGStreamOutConfig;

class CGPipeline {

	// Shader interfaces
	ID3D11VertexShader				*vertexShader;
	ID3D11GeometryShader			*geometryShader;
	ID3D11PixelShader				*pixelShader;

	// Rasteriser configuration
	CGRasteriserStage				*RSStage; // weak reference to the rasteriser state object

	// Output merger configuration
	CGOutputMergerStage				*OMStage; // weak reference to output merger state object
	

public:

	// Configure a rendering pipeline given the appropriate shader interfaces and fixed stage configuration models.  No shaders are compiled here so this overload of the constructor does not return and compiled shader bytecode.  Also, we don't provide any stream-out configuration here since the GS is already built.
	CGPipeline(
		ID3D11Device *device,
		ID3D11VertexShader *vs,
		ID3D11GeometryShader *gs,
		ID3D11PixelShader *ps,
		CGRasteriserStage *_rsStage,
		CGOutputMergerStage *_omStage);

	// Configure a rendering pipeline given path names to shader files (typically .hlsl) and fixed stage configuration models.  Return the compiled vertex shader bytecode if a valid ID3DBlob pointer is given.  This is used for IA input layout creation for a given vertex structure used in the loaded vertex shader.  If the ID3DBlob pointer is a nullptr then we don't need the bytecode since this is likely to have been created before (only one input layout is needed for a given vertex structure)
	CGPipeline(
		ID3D11Device *device,
		const char* vsPath,
		const char* gsPath,
		const char* psPath,
		CGStreamOutConfig *soConfig,
		CGRasteriserStage *_rsStage,
		CGOutputMergerStage *_omStage,
		ID3DBlob **vsBytecode=nullptr);

	~CGPipeline();

	bool applyPipeline(ID3D11DeviceContext *context);
};
