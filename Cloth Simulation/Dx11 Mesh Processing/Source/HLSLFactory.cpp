
#include "HLSLFactory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "D3Dcompiler.h"
#include "CGVertexExt.h"

using namespace std;


//
// Private functions
//

static void printSourceListing(const string& sourceString, bool showLineNumbers=true) {

	const char *srcPtr = sourceString.c_str();
	const char *srcEnd = srcPtr + sourceString.length();
	
	size_t lineIndex = 0;

	while (srcPtr < srcEnd) {

		if (showLineNumbers) {

			cout.fill(' ');
			cout.width(4);
			cout << dec << ++lineIndex << " > ";
		}

		size_t substrLength = strcspn(srcPtr, "\n");
		
		cout << string(srcPtr, 0, substrLength) << endl;

		srcPtr += substrLength + 1;
	}
}


static string *shaderSourceStringFromFile(const string& filePath) {

	string *sourceString = NULL;

	const char *file_str = filePath.c_str();

	struct stat fileStatus;
	int file_error = stat(file_str, &fileStatus);

	if (file_error==0) {

		_off_t fileSize = fileStatus.st_size;

		char *src = (char *)calloc(fileSize+1, 1); // add null-terminator character at end of string

		if (src) {

			ifstream shaderFile(file_str);

			if (shaderFile.is_open()) {

				shaderFile.read(src, fileSize);
				sourceString = new string(src);

				shaderFile.close();
			}

			// dispose of local resources
			free(src);
		}
	}

	// return pointer to new source string
	return sourceString;
}


//
// Public interface
//

HRESULT HLSLFactory::loadVertexShader(ID3D11Device *device, const std::string& filepath, ID3D11VertexShader **vertexShaderInterface, ID3DBlob **vertexShaderBytecode) {

	return loadVertexShader(device, filepath, string("vertexShader"), vertexShaderInterface, vertexShaderBytecode);
}


HRESULT HLSLFactory::loadVertexShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, ID3D11VertexShader **vertexShaderInterface, ID3DBlob **vertexShaderBytecode) {

	string				*vsSource = nullptr;
	ID3D11VertexShader	*shader = nullptr;
	ID3DBlob			*bytecode = nullptr;
	ID3DBlob			*vsErrorObject = nullptr;
	
	try
	{
		vsSource = shaderSourceStringFromFile(filepath);
		
		if (!vsSource)
			throw("Cannot load the vertex shader HLSL file");

		// Compile the vertex shader source contained in vsSource
		HRESULT hr = D3DCompile(vsSource->c_str(), vsSource->length(), NULL, NULL, NULL , shaderFunctionName.c_str(), "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &bytecode, &vsErrorObject);

		// Check and report compilation errors
		if (!SUCCEEDED(hr)) {

			if (vsErrorObject) {

				cout << "The vertex shader \"" << filepath << "\" could not be compiled successfully...\n\n";
				cout << "Report:\n\nVertex shader source code...\n\n";
				
				printSourceListing(*vsSource, true);
				
				// Report compilation error log
				cout << "\n<vertex shader compiler errors--------------------->\n\n";
				cout << (char*)(vsErrorObject->GetBufferPointer());
				cout << "\n<-----------------end vertex shader compiler errors>\n\n\n";
			}

			throw("Vertex shader compile error");
		}


		// Create the vertex shader object
		hr = device->CreateVertexShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, &shader);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create the vertex shader interface");


		// Cleanup
		
		if (vsSource)
			delete vsSource;

		if (vsErrorObject)
			vsErrorObject->Release();

		*vertexShaderInterface = shader;
		*vertexShaderBytecode = bytecode;
	}
	catch (char *)
	{
		// Cleanup
		if (vsSource)
			delete vsSource;

		if (shader)
			shader->Release();

		if (bytecode)
			bytecode->Release();

		if (vsErrorObject)
			vsErrorObject->Release();

		// Re-throw exception
		throw;
	}

	return S_OK;
}



HRESULT HLSLFactory::loadPixelShader(ID3D11Device *device, const std::string& filepath, ID3D11PixelShader **shader) {

	return loadPixelShader(device, filepath, string("pixelShader"), shader);
}


HRESULT HLSLFactory::loadPixelShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, ID3D11PixelShader **shader) {

	string				*psSource = nullptr;
	ID3D11PixelShader	*pixelShader = nullptr;
	ID3DBlob			*shaderBlob = nullptr;
    ID3DBlob			*errorBlob = nullptr;

	try
	{
		psSource = shaderSourceStringFromFile(filepath);

		if (!psSource)
			throw("Cannot load the pixel shader HLSL file");
		
		// Compile the pixel shader source contained in psSource
		HRESULT hr = D3DCompile(psSource->c_str(), psSource->length(), NULL, NULL, NULL, shaderFunctionName.c_str(), "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &shaderBlob, &errorBlob);

		// Check and report compilation errors
		if (!SUCCEEDED(hr)) {

			if (errorBlob) {

				cout << "The pixel shader \"" << filepath << "\" could not be compiled successfully...\n\n";
				cout << "Report:\n\nPixel shader source code...\n\n";
				
				printSourceListing(*psSource, true);
				
				// Report compilation error log
				cout << "\n<Pixel shader compiler errors--------------------->\n\n";
				cout << (char*)(errorBlob->GetBufferPointer());
				cout << "\n<-----------------end pixel shader compiler errors>\n\n\n";
			}

			throw("Pixel shader compile error");
		}

		// Create the pixel shader object
		hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pixelShader);
		
		if (!SUCCEEDED(hr))
			throw("Cannot create the pixel shader interface");

		// Cleanup

		if (psSource)
			delete psSource;

		if (errorBlob)
			errorBlob->Release();

		if (shaderBlob)
			shaderBlob->Release();

		*shader = pixelShader;
	}
	catch (char *)
	{
		// Cleanup
		if (psSource)
			delete psSource;

		if (pixelShader)
			pixelShader->Release();

		if (errorBlob)
			errorBlob->Release();

		if (shaderBlob)
			shaderBlob->Release();

		// Re-throw exception
		throw;
	}

	return S_OK;
}



HRESULT HLSLFactory::loadGeometryShader(ID3D11Device *device, const std::string& filepath, CGStreamOutConfig *soConfig, ID3D11GeometryShader **shader) {

	return loadGeometryShader(device, filepath, string("geometryShader"), soConfig, shader);
}


HRESULT HLSLFactory::loadGeometryShader(ID3D11Device *device, const std::string& filepath, const std::string& shaderFunctionName, CGStreamOutConfig *soConfig, ID3D11GeometryShader **shader) {

	string					*gsSource = nullptr;
	ID3D11GeometryShader	*geometryShader = nullptr;
	ID3DBlob				*shaderBlob = nullptr;
    ID3DBlob				*errorBlob = nullptr;

	try
	{
		gsSource = shaderSourceStringFromFile(filepath);

		if (!gsSource)
			throw("Cannot load the geometry shader HLSL file");
		
		// Compile the geometry shader source contained in gsSource
		HRESULT hr = D3DCompile(gsSource->c_str(), gsSource->length(), NULL, NULL, NULL, shaderFunctionName.c_str(), "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &shaderBlob, &errorBlob);

		// Check and report compilation errors
		if (!SUCCEEDED(hr)) {

			if (errorBlob) {

				cout << "The geometry shader \"" << filepath << "\" could not be compiled successfully...\n\n";
				cout << "Report:\n\nGeometry shader source code...\n\n";
				
				printSourceListing(*gsSource, true);
				
				// Report compilation error log
				cout << "\n<Geometry shader compiler errors--------------------->\n\n";
				cout << (char*)(errorBlob->GetBufferPointer());
				cout << "\n<-----------------end geometry shader compiler errors>\n\n\n";
			}

			throw("Geometry shader compile error");
		}

		
		// Create the geometry shader object.  If soConfig is provided then configure the geometry shader with stream-out capability

		if (!soConfig)
			hr = device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &geometryShader);
		else
			hr = device->CreateGeometryShaderWithStreamOutput(
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(),
			soConfig->streamOutDeclaration,
			soConfig->streamOutSize,
			soConfig->streamOutVertexStrides,
			soConfig->numVertexStrides,
			soConfig->rasteriseStreamIndex,
			nullptr,
			&geometryShader);


		if (!SUCCEEDED(hr))
			throw("Cannot create the geometry shader interface");

		// Cleanup

		if (gsSource)
			delete gsSource;

		if (errorBlob)
			errorBlob->Release();

		if (shaderBlob)
			shaderBlob->Release();

		*shader = geometryShader;
	}
	catch (char *)
	{
		// Cleanup
		if (gsSource)
			delete gsSource;

		if (geometryShader)
			geometryShader->Release();

		if (errorBlob)
			errorBlob->Release();

		if (shaderBlob)
			shaderBlob->Release();

		// Re-throw exception
		throw;
	}

	return S_OK;
}
