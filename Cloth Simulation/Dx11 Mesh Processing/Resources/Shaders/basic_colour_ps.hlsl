
// Basic colour pixel shader


// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {

	float4				colour		: COLOR;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};



fragmentOutputPacket pixelShader(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	outputFragment.fragmentColour = inputFragment.colour;

	return outputFragment;
}
