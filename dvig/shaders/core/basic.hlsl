cbuffer buffer
{
    float4x4 perspectiveProj;
};

struct vs_in
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct vs_out
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

vs_out VShader(vs_in vsIn)
{
    vs_out vsOut;
    vsOut.pos = mul(perspectiveProj, vsIn.pos);
    vsOut.color = vsIn.color;

    return vsOut;
}

float4 PShader(vs_out input) : SV_TARGET
{
    return input.color;
}
