struct VertexInput
{
    float3 position : ATTR0;
    float2 uv : ATTR1;
};

struct Varyings
{
    float3 positionCS : SV_Position;
    float3 positionWS : POSITION;
    float2 uv0 : TEXCOORD0;
};

cbuffer Object
{
    float4x4 model;
    float4x4 viewProjection;
};

Varyings Vertex(VertexInput input)
{
    Varyings output;
    output.positionWS = mul(float4(input.position, 1.0), model).xyz;
    output.positionCS = mul(float4(output.positionWS, 1.0), viewProjection);
    output.uv0 = input.uv;

    return output;
}

float4 Pixel(Varyings input) : SV_Target
{
    return float4(input.uv0, 0.0, 1.0);
}