
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


cbuffer camera : register(b0) {

	float4x4			viewProjMatrix;
	float3				eyePos;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct vertexInputPacket {

    float3				pos			: POSITION;
    float4				colour		: COLOR;
};


struct vertexOutputPacket {

	float4				colour		: COLOR;
	float4				posH		: SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
vertexOutputPacket vertexShader(vertexInputPacket inputVertex) {

	vertexOutputPacket outputVertex;

	outputVertex.colour = inputVertex.colour;
	outputVertex.posH = mul(float4(inputVertex.pos, 1.0), viewProjMatrix);
	
	return outputVertex;
}
