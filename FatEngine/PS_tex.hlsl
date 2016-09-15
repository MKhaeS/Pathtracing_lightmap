Texture2D t1 : register(t0);
Texture2D t2 : register(t0);
SamplerState s1 : register(s0);

struct VSout
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};


float4 PS (VSout vsout) : SV_TARGET{
	float4 col = t1.Sample (s1, vsout.uv);
	return col;
}
