
// Basic colour pixel shader for Hermite curve interpolation


// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {

	float4				colour		: COLOR;
	float2				texCoord	: TEXCOORD;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};



fragmentOutputPacket pixelShader(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	outputFragment.fragmentColour = inputFragment.colour;

	return outputFragment;
}
