
//
// Pass-through vertex shader for the snow particle system demo.  This is used for both the update and render pipeline configurations.  Billboarding will be done in eye / view coordinate space, but there is no loss of efficiency in converting particles to eye / view coordinates in the geometry shader since the input primitive is only points - there is no duplication of vertex processing in this example.
//

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct throughVertexPacket {

    float3				pos				: POSITION;
	float3				velocity		: VELOCITY;
	float				weight			: WEIGHT;
	float				age				: AGE;
	float				theta			: ORIENTATION;
	float				angularVelocity	: ANGULARVELOCITY;
	uint				isaGenerator	: GENERATOR;
	uint				flakeType		: FLAKETYPE; // determine which texture will be used [0, 7]
};


//--------------------------------------------------------------------------------------
// Vertex Shader (Simple pass-through shader)
//--------------------------------------------------------------------------------------
throughVertexPacket vertexShader(throughVertexPacket inputVertex)
{	
	return inputVertex;
}
