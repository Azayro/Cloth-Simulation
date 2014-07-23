
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct throughVertexPacket {

    float3				pos			: POSITION;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader (Simple pass-through shader)
//--------------------------------------------------------------------------------------
throughVertexPacket vertexShader(throughVertexPacket inputVertex)
{	
	return inputVertex;
}
