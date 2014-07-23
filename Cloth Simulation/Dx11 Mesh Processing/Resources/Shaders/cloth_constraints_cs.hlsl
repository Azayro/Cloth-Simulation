
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
RWStructuredBuffer<Particle> particles : register(u0);

struct Constraint
{
	uint start;
	uint end;

	float length;
};
StructuredBuffer<Constraint> constraints : register(t0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	// Set the particles where the constraints starts and ends
	Particle particleOne = particles[constraints[DTid.x].start];
	Particle particleTwo = particles[constraints[DTid.x].end];

	// Find the delta of the particles
	float3 delta = particleOne.pos - particleTwo.pos;

	// Get the distance between the particles
	float distance = length(delta);
	float stretching = 1 - constraints[DTid.x].length / distance;
	delta *= stretching;

	particles[constraints[DTid.x].start].pos -= (delta * 0.5);
	particles[constraints[DTid.x].end].pos += (delta * 0.5);
}