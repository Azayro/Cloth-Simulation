
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


cbuffer camera : register(b0) {

	float4x4			viewProjMatrix;
	float3				eyePos;
};

cbuffer gameTime : register(b1) {

	float				gameTime;
};

cbuffer worldTransform : register(b2) {

	float4x4			worldMatrix;
	float4x4			normalMatrix; // inverse transpose of worldMatrix
};



//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct vertexInputPacket {

    float3				pos			: POSITION;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
};


struct vertexOutputPacket {

	float3				posW		: POSITION; // vertex in world coords
	float3				normalW		: NORMAL; // normal in world coords
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
vertexOutputPacket vertexShader(vertexInputPacket inputVertex)
{
	vertexOutputPacket outputVertex;

	float4x4 wvp = mul(worldMatrix, viewProjMatrix);
	
	// animate apex in model coord space
	float4 pos = float4(inputVertex.pos, 1.0);

	outputVertex.posH = mul(pos, wvp);
	outputVertex.posW = mul(pos, worldMatrix).xyz;
	
	 // multiply the input normal by the inverse-transpose of the world transform matrix
	outputVertex.normalW = mul(float4(inputVertex.normal, 1.0), normalMatrix).xyz;
	
	outputVertex.matDiffuse = inputVertex.matDiffuse;
	outputVertex.matSpecular = inputVertex.matSpecular;
	
	outputVertex.texCoord = inputVertex.texCoord;

	return outputVertex;
}
