

struct Vin
{
	float3 pos : POSITION;
	float2 uv  : TEXCOORD;
};

struct VSout
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD;
};




VSout VS (Vin vin) {
	VSout vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.uv = vin.uv;

	return vout;
}