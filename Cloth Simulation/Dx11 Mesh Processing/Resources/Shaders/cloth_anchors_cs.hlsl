struct Particle
{
	// CGVertexExt
	float3		pos			: POSITION;
	float3		normal		: NORMAL;
	uint		matDiffuse	: DIFFUSE;
	uint		matSpecular	: SPECULAR;
	float2		texCoord	: TEXCOORD;

	float3 prevPos;
};
RWStructuredBuffer<Particle> particles	: register(u0);

struct Anchor
{
	uint index;

	float3 pos;
};
StructuredBuffer<Anchor> anchors : register(t1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	// Set anchor points
	particles[anchors[DTid.x].index].pos = anchors[DTid.x].pos;
}