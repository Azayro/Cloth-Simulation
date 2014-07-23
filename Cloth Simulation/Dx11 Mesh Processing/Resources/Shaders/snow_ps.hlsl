
// Basic texture-lookup pixel shader for rendering the snow particle system


Texture2DArray			snowflakeArray : register(t1);
SamplerState			linearSampler : register(s0);


// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {

	float4				colour		: COLOR;
	float2				texCoord	: TEXCOORD;
	uint				id			: FLAKETYPE;
	float				lDepth		: LINEAR_DEPTH;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};



fragmentOutputPacket pixelShader(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	outputFragment.fragmentColour = snowflakeArray.Sample(linearSampler, float3(inputFragment.texCoord, inputFragment.id % 8));

	return outputFragment;
}
