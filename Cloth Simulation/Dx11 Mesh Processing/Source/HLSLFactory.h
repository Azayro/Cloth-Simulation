#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <string>


// Structure used to describe the SO stage when creating a geometry shader with Stream-out capability
struct CGStreamOutConfig {

	UINT							streamOutSize; // number of entries in streamOutDeclaration
	D3D11_SO_DECLARATION_ENTRY		*streamOutDeclaration;
	
	UINT							numVertexStrides; // number of stride entires in streamOutVertexStrides
	UINT							*streamOutVertexStrides;

	UINT							rasteriseStreamIndex;
};


class HLSLFactory {

public:

	// Load and compile a given vertex shader file - the name of the shader function defaults to "vertexShader"
	static HRESULT loadVertexShader(ID3D11Device *device, const std::string& filepath, ID3D11VertexShader **vertexShaderInterface, ID3DBlob **vertexShaderBytecode);

	// Overload of loadVertexShader that allows the name of the shader function to be specified in the string shaderFunctionName
	static HRESULT loadVertexShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, ID3D11VertexShader **vertexShaderInterface, ID3DBlob **vertexShaderBytecode);

	// Load and compile a given pixel shader file - the name of the shader function defaults to "pixelShader"
	static HRESULT loadPixelShader(ID3D11Device *device, const std::string& filepath, ID3D11PixelShader **shader);

	// Overload of loadPixelShader that allows the name of the shader function to be specified in the string shaderFunctionName
	static HRESULT loadPixelShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, ID3D11PixelShader **shader);


	static HRESULT loadGeometryShader(ID3D11Device *device, const std::string& filepath, CGStreamOutConfig *soConfig, ID3D11GeometryShader **shader);

	static HRESULT loadGeometryShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, CGStreamOutConfig *soConfig, ID3D11GeometryShader **shader);

};
