
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------


// Ensure matrices are row-major
#pragma pack_matrix(row_major)


cbuffer cameraPosition : register(b0) {

	float4x4			viewMatrix;
};

cbuffer cameraProjection : register(b2) {

	float4x4			projMatrix;
	float				nearPlane;
	float				farPlane;
};

cbuffer snowSystemUpdateConstants : register (b5) {

	float4		gravity;
	float		resetAge;
	float		ageDelta;
	float		generatorAgeThreshold;
	float		particleDestroyAge;
	
	// Pseudo-random numbers (x, y, z, w) generated for current pass that lie in the range [-1, +1) - okay for now since we're unlikely to generate more than one new particle for each update cycle.  (x, y, z) are used for initial velocity values while w is used to initialise the new particle age
	float4		prn;
	
	// Pesudo-random number in the range [0, 8) to determine the initial particle type.  Again, only a single new particle likely in every update cycle  (see above)
	uint		randomFlakeType;
};



//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------

struct inputParticleStructure {

    float3				pos				: POSITION;
	float3				velocity		: VELOCITY;
	float				weight			: WEIGHT;
	float				age				: AGE;
	float				theta			: ORIENTATION;
	float				angularVelocity	: ANGULARVELOCITY;
	uint				isaGenerator	: GENERATOR;
	uint				flakeType		: FLAKETYPE; // determine which texture will be used [0, 7]
};

struct vertexOutputPacket {

	float4				colour		: COLOR;
	float2				texCoord	: TEXCOORD;
	uint				id			: FLAKETYPE;
	float				lDepth		: LINEAR_DEPTH;
	float4				posH		: SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------

// Each particle is expanded into a screen-aligned quad in view coordinate space.  This is a more convinient coordinate space to do billboarding rather in world coordinate space.
[maxvertexcount(4)]
void geometryShader(
	point inputParticleStructure inputParticle[1],
	inout TriangleStream<vertexOutputPacket> outputQuadStream)
{
	vertexOutputPacket		outputVertex;
	
	if (inputParticle[0].isaGenerator==0) {
		
		// Calculate coordinate of vertex in eye / view coordinate space
		float4 posE = mul(float4(inputParticle[0].pos, 1.0), viewMatrix);

		float hwidth = 0.25;
		float hheight = 0.25;
		
		float cosTheta = cos(inputParticle[0].theta);
		float sinTheta = sin(inputParticle[0].theta);
		
		float4x4 R = {cosTheta, sinTheta, 0.0, 0.0,
					  -sinTheta, cosTheta, 0.0, 0.0,
					  0.0, 0.0, 1.0, 0.0,
					  0.0, 0.0, 0.0, 1.0};
		
		float4 p1 = posE + mul(float4(hwidth, -hheight, 0.0, 0.0), R);
		float4 p2 = posE + mul(float4(-hwidth, -hheight, 0.0, 0.0), R);
		float4 p3 = posE + mul(float4(hwidth, hheight, 0.0, 0.0), R);
		float4 p4 = posE + mul(float4(-hwidth, hheight, 0.0, 0.0), R);
		
		// Colour modulation based on particle age
		float ageCoeff = 1.0 - (inputParticle[0].age / particleDestroyAge);
		
		outputVertex.colour = float4(ageCoeff, ageCoeff, ageCoeff, 0.4);
		outputVertex.lDepth = (posE.z - nearPlane) / (farPlane - nearPlane); // Linearised z-depth
		outputVertex.id = inputParticle[0].flakeType;
		
		// Note: z depth constant since quad is screen aligned (all z coords in eye / view space are the same)
		// Note 2: so is primitiveID
		
		outputVertex.posH = mul(p1, projMatrix);
		outputVertex.texCoord = float2(1.0, 1.0);
		outputQuadStream.Append(outputVertex);
		
		outputVertex.posH = mul(p2, projMatrix);
		outputVertex.texCoord = float2(0.0, 1.0);
		outputQuadStream.Append(outputVertex);
		
		outputVertex.posH = mul(p3, projMatrix);
		outputVertex.texCoord = float2(1.0, 0.0);
		outputQuadStream.Append(outputVertex);
		
		outputVertex.posH = mul(p4, projMatrix);
		outputVertex.texCoord = float2(0.0, 0.0);
		outputQuadStream.Append(outputVertex);
	}
}
