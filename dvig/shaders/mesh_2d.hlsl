struct VsIn {
    float2 pos : POSITION;
};

cbuffer ConstBuffer {
    float4 color;
    float4x4 transform;
};

struct VsOut {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VsOut vertex_main(VsIn vs_in) {
    VsOut vs_out;
    vs_out.pos = mul(transform, float4(vs_in.pos.x, vs_in.pos.y, 0, 1));
    vs_out.color = color;
    return vs_out;
}

float4 pixel_main(VsOut input) : SV_TARGET {
    return input.color;
}
