
//
// Model the update of each particle.  Each particle is modelled in world coordinate space
//


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

// For the update process, the geometry shader (GS) inputs and outputs the same vertex structure
struct particleStructure {

    float3				pos				: POSITION;
	float3				velocity		: VELOCITY;
	float				weight			: WEIGHT;
	float				age				: AGE;
	float				theta			: ORIENTATION;
	float				angularVelocity	: ANGULARVELOCITY;
	uint				isaGenerator	: GENERATOR;
	uint				flakeType		: FLAKETYPE; // determine which texture will be used [0, 7]
};




// Output a maximum of 2 particles (current particle + possible new particle (if current particle is a generator and time has passed to create a new particle)
[maxvertexcount(2)]
void geometryShader(
	point particleStructure inputParticle[1],
	inout PointStream<particleStructure> outputParticleStream)
{
	particleStructure			outputParticle;
	
	if (inputParticle[0].isaGenerator == 1) {

		//
		// stream out geneator / emitter with a new age value
		//
		outputParticle = inputParticle[0];

		if (inputParticle[0].age >= generatorAgeThreshold) {

			// need to reset age
			outputParticle.age = resetAge;
		
		} else {

			// countdown generator age to create new particle
			outputParticle.age = inputParticle[0].age + ageDelta;
		}
				
		outputParticleStream.Append(outputParticle);
		
		//
		// Create new particle if necessary
		//
		if (inputParticle[0].age >= generatorAgeThreshold) {

			outputParticle.pos = inputParticle[0].pos; // new particles start at same location as emitter
			outputParticle.velocity = float3(prn.x*0.01, 0.0, prn.z*0.01);
			outputParticle.weight = 1.0;
			outputParticle.age = prn.w; // initialise particle age (will count down normal particle age)
			outputParticle.theta = 0.0;
			outputParticle.angularVelocity = prn.y * (3.142 / 180.0);
			outputParticle.isaGenerator = 0;
			outputParticle.flakeType = randomFlakeType;
			
			outputParticleStream.Append(outputParticle);
		}
	
	} else {

		// Modify normal particle - don't emit if age reaches particleDestroyAge
		
		if (inputParticle[0].age > 0.0) {

			outputParticle.pos = inputParticle[0].pos + (inputParticle[0].velocity * 0.05);
			outputParticle.velocity = inputParticle[0].velocity + gravity.xyz;
			outputParticle.weight = inputParticle[0].weight;
			outputParticle.age = inputParticle[0].age - ageDelta;
			outputParticle.theta = inputParticle[0].theta + (inputParticle[0].angularVelocity * 0.25);
			outputParticle.angularVelocity = inputParticle[0].angularVelocity;
			outputParticle.isaGenerator = inputParticle[0].isaGenerator;
			outputParticle.flakeType = inputParticle[0].flakeType;

			outputParticleStream.Append(outputParticle);
		}	
	}
}
