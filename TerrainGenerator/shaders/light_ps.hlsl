// Basic lighting and texture pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float3 lightDirection;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 texCol = tex0.Sample(sampler0, input.tex); 
    float4 lightCol = calculateLighting(-lightDirection, input.normal, diffuseColour);
    
    float4 finalCol = saturate(lightCol) * texCol;
    
    return finalCol;
}


