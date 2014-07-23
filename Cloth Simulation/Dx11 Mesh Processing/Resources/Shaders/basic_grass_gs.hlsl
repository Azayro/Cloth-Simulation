
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


cbuffer camera : register(b0) {

	float4x4			viewProjMatrix;
	float3				eyePos;
};

cbuffer environment : register(b1) {

	float				windPhase;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct inputVertexPacket {

    float3				pos			: POSITION;
	float4				colour		: COLOR;
	float				weight		: WEIGHT;
};

struct outputVertexPacket {
	
	float4				colour		: COLOR;
	float4				posH		: SV_POSITION;
};



//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(5)]
void geometryShader(
	point inputVertexPacket gIn[1],
	inout LineStream<outputVertexPacket> lStream)
{
	outputVertexPacket outputVertex;
	
	outputVertex.colour = gIn[0].colour;
	outputVertex.posH = mul(float4(gIn[0].pos, 1.0), viewProjMatrix);
	lStream.Append(outputVertex);
	
	outputVertex.posH = mul(float4(gIn[0].pos + float3(windPhase*0.01*(1.0-gIn[0].weight), 0.2, 0.0), 1.0), viewProjMatrix);
	lStream.Append(outputVertex);
	
	outputVertex.posH = mul(float4(gIn[0].pos + float3(windPhase*0.04*(1.0-gIn[0].weight), 0.5, 0.0), 1.0), viewProjMatrix);
	lStream.Append(outputVertex);
	
	lStream.RestartStrip();
	
	outputVertex.posH = mul(float4(gIn[0].pos + float3(windPhase*0.01*(1.0-gIn[0].weight), 0.2, 0.0), 1.0), viewProjMatrix);
	lStream.Append(outputVertex);
	
	outputVertex.posH = mul(float4(gIn[0].pos + float3(windPhase*0.01*(1.0-gIn[0].weight)+0.01, 0.25, 0.1), 1.0), viewProjMatrix);
	lStream.Append(outputVertex);
	
	
	
	/*outputVertex.posH = mul(mul(float4(gIn[0].posL + float3(0.1, 0.0, -0.2), 1.0), viewMatrix), projectionMatrix);
	lStream.Append(outputVertex);
	
	outputVertex.posH = mul(mul(float4(gIn[0].posL + float3(0.1, 0.2, -0.2), 1.0), viewMatrix), projectionMatrix);
	lStream.Append(outputVertex);
	
	outputVertex.posH = mul(mul(float4(gIn[0].posL + float3(0.1, 0.5, -0.2), 1.0), viewMatrix), projectionMatrix);
	lStream.Append(outputVertex);
	*/
}
