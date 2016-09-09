struct VSout
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};


float4 PS (VSout vsout) : SV_TARGET {
    float4 col = vsout.col;
    return col;
}
