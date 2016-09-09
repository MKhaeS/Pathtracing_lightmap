

struct Vin
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct VSout
{
    float4 pos :SV_POSITION;
    float4 col : COLOR;
};




VSout VS ( Vin vin)  {
    VSout vout;
    vout.pos = float4(vin.pos, 1.0f);
    vout.col = vin.col;

    return vout;
}