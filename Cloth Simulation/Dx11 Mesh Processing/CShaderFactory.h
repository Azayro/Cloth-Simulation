#include <d3dx11.h>
#include <d3dcompiler.h>

class CShaderFactory
{
public:
	static HRESULT CompileComputeShader( _In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ ID3D11Device* device, _Out_ ID3DBlob** blob );
};