

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

struct Matrix
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4x4 dummy;
};

ConstantBuffer<Matrix>  mtxbuff : register (b0);


VSout VS (Vin vin) {
	VSout vout;
	vout.pos = mul(float4(vin.pos, 1.0f), mtxbuff.world);
	vout.uv = vin.uv;

	return vout;
}