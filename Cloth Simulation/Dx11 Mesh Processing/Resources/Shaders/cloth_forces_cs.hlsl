
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
	/*float3 pos = particles[DTid.x].pos;

	pos = pos + (particles[DTid.x].normal * -0.0001);

	particles[DTid.x].pos = pos;*/

	float3 force = float3(0, -1.0, -1.0);
	float3 velocity = ((particles[DTid.x].pos * 2) - particles[DTid.x].prevPos);

	float3 nextPos = (velocity * 0.001) + 0.5 * (force * 0.001);

	particles[DTid.x].prevPos = particles[DTid.x].pos;

	particles[DTid.x].pos += nextPos;

}