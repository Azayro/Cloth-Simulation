
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
struct inputVertexPacket {

	float3				pos			: POSITION;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
};

struct outputVertexPacket {
	
	float4				colour		: COLOR;
	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION;
};




//--------------------------------------------------------------------------------------
// Hermite spline functions
//--------------------------------------------------------------------------------------

// Model Hermite curves for each line segment - use adjacency information to calculate tangent vectors at ends of curve.

// H() returns the Hermite coefficients for t
float4 H(float t)
{
	float t2 = t * t;
	float t3 = t2 * t;
	float tm = t - 1;
	
	float h1 = 1.0 - 3.0 * t2 + 2.0 * t3;
	float h2 = t2 * (3.0 - (t + t));
	float h3 = t * (tm * tm);
	float h4 = t2 * tm;
	
	return float4(h1, h2, h3, h4);
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(11)]
void geometryShader(
	lineadj inputVertexPacket inputLineAdj[4],
	inout LineStream<outputVertexPacket> outputLineStream)
{
	outputVertexPacket			outputVertex;
	
	float3 v;
	
	float3 P1 = inputLineAdj[1].pos;
	float3 P2 = inputLineAdj[2].pos;
	float3 T1 = inputLineAdj[2].pos - inputLineAdj[0].pos;
	float3 T2 = inputLineAdj[3].pos - inputLineAdj[1].pos;
	
	float t = 0.0;
	for (int i=0;i<=10;i++)
	{
		float4 h = H(t);
		
		v = (P1 * h.x) + (P2 * h.y) + (T1 * h.z) + (T2 * h.w);
		
		outputVertex.colour = (inputLineAdj[1].matDiffuse * t) + (inputLineAdj[2].matDiffuse * (1.0 - t));
		outputVertex.texCoord = (inputLineAdj[1].texCoord * t) + (inputLineAdj[2].texCoord * (1.0 - t));
		outputVertex.posH = mul(float4(v, 1.0), viewProjMatrix);

		outputLineStream.Append(outputVertex);

		t += 0.1;
	}
}
