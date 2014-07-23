
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


// Light model (cut down version of Luna's model (p.197) since we're only interested in point lights for now
struct CGLight {

	float3				pos;
	float1				range; // radius of sphere in which light applied
	float4				ambient;
	float4				diffuse;
	float4				specular; // specular.a represents the specular power component
	float3				attenuation; // constant, linear, quadratic
};


// Model a given surface point from the fragment shader to pass to lighting calculation functions
struct SurfacePoint {

	float3				pos;
	float2				texCoord;
	float3				normal;
	float4				matDiffuse;
	float4				matSpecular;
};




//
// constant buffer declarations
//

cbuffer camera : register(b0) {

	float4x4			viewProjMatrix;
	float3				eyePos;
};

cbuffer lightModel : register(b1) {

	CGLight				demoPointLight;
};


//
// Textures and samplers
//

Texture2D				myTexture : register(t0);
SamplerState			linearSampler : register(s0);


//--------------------------------------------------------------------------------------
// Lighting functions
//--------------------------------------------------------------------------------------

// Calculate colour / brightness at surface point v given light source L
float3 pointLight(SurfacePoint v, CGLight L, float3 eyePos) {

	float3 lightVec = L.pos - v.pos;
	float d = length(lightVec);
	
	lightVec /= d;
	
	// Sample texture to obtain surface diffuse colour
	float3 surfaceDiffuseColour = myTexture.Sample(linearSampler, v.texCoord).rgb;
	
	// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
	float dp = clamp(dot(lightVec, v.normal), 0.0, 1.0);

	// Calculate the ambient light component
	float3 ambient = L.ambient.rgb * surfaceDiffuseColour;

	// Calculate the diffuse light component
	float3 diffuse = dp * L.diffuse.rgb * surfaceDiffuseColour;
	
	// Calculate the specular light component
	float3 toEye = normalize(eyePos - v.pos);
	float3 R = reflect(-lightVec, v.normal);
	float specFactor = pow(max(dot(R, toEye), 0.0f), L.specular.a);
	float3 specular = ceil(dp) * specFactor * L.specular.rgb * v.matSpecular.rgb;

	// Calculate and return the final colour (with attenuation term to scale brightness)
	return 1.0 / dot(L.attenuation, float3(1.0f, d, d*d)) * (ambient + diffuse + specular);
}



//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------

// input fragment - this is the per-fragment packet interpolated by the rasteriser stage.  SV_POSITION is consumed by the pipeline and not input here
struct fragmentInputPacket {

	float3				posW		: POSITION; // vertex in world coords
	float3				normalW		: NORMAL; // normal in world coords
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//--------------------------------------------------------------------------------------
// Fragment Shader
//--------------------------------------------------------------------------------------
fragmentOutputPacket pixelShader(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	float3 N = normalize(inputFragment.normalW);
	SurfacePoint v = {inputFragment.posW, inputFragment.texCoord, N, inputFragment.matDiffuse, inputFragment.matSpecular};
	
	float3 color = pointLight(v, demoPointLight, eyePos);
	
	outputFragment.fragmentColour = float4(color, 1.0);	

	return outputFragment;
}
