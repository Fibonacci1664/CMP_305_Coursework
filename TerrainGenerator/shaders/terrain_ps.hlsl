// Terrain pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D snowTex : register(t0);
Texture2D grassTex : register(t1);
Texture2D waterTex : register(t2);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float3 lightDirection;
    float padding;
};

cbuffer NoiseStyleBuffer : register(b1)
{
    float noiseStyle;
    float3 noisePadding;
};

cbuffer TextureBoundsBuffer : register(b2)
{
    // N = for normal height texturing
    float N_waterLowerBound;
    float N_waterUpperbound;
    float N_grassLowerBound;
    float N_grassUpperBound;
    
    // R = for ridged height texturing
    float R_waterLowerBound;
    float R_waterUpperbound;
    float R_grassLowerBound;
    float R_grassUpperBound;
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

float4 doNormalHeightTexturing(float height, float4 snowTexCol, float4 grassTexCol, float4 waterTexCol, float4 lightCol)
{
    float4 blendTex = 0.0f;
    float lowerBound = 0.0f;        // a1
    float upperBound = 0.0f;        // a2
    float lowerLerpCoeff = 0.0;     // b1
    float upperLerpCoeff = 1.0f;    // b2
    float lerpCoeff = 0.0f;
   
    float heightNorm = normalize(height);
    
    // This is a max negative value to ensure that any height that has been displaced downwards
    // is textured with water, otherwise we run the risk of not meeting the conditions of each of these
    // statements and therefore end up texturing with snow, the last return statement
    lowerBound = N_waterLowerBound; // a1
    upperBound = N_waterUpperbound; // a2

    //                     a1    a2                      b1  b2  
    // Map boundary range [val, val] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // If at the boundary of this current tex and the next one, then blend between them
    // blend between water and grass
    if (height <= 0.0f || (height >= lowerBound && height <= upperBound))
    {
        blendTex = lerp(waterTexCol, grassTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    lowerBound = N_grassLowerBound; // a1
    upperBound = N_grassUpperBound; // a2
       
    //                     a1    a2                      b1  b2  
    // Map boundary range [val, val] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // Blend between grass and snow
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(grassTexCol, snowTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    // Otherwise just texture snow
    return saturate(lightCol) * snowTexCol;
}

float4 doRidgedHeightTexturing(float height, float4 snowTexCol, float4 grassTexCol, float4 waterTexCol, float4 lightCol)
{
    float4 blendTex = 0.0f;
    float lowerBound = 0.0f; // a1
    float upperBound = 0.0f; // a2
    float lowerLerpCoeff = 0.0; // b1
    float upperLerpCoeff = 1.0f; // b2
    float lerpCoeff = 0.0f;
   
    float heightNorm = normalize(height);
    
    lowerBound = R_waterLowerBound; // a1
    upperBound = R_waterUpperbound; // a2

    //                     a1    a2                      b1  b2  
    // Map boundary range [val, val] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // If at the boundary of this current tex and the next one, then blend between them
    // blend between water and grass
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(waterTexCol, grassTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    lowerBound = R_grassLowerBound; // a1
    upperBound = R_grassUpperBound; // a2
       
    //                      a1    a2                      b1  b2  
    // Map boundary range [-5.0, -0.5] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // Blend between grass and snow
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(grassTexCol, snowTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    // Otherwise just texture snow
    return saturate(lightCol) * snowTexCol;
}

float4 main(InputType input) : SV_TARGET
{
    float4 snowTexCol = snowTex.Sample(sampler0, input.tex);
    float4 grassTexCol = grassTex.Sample(sampler0, input.tex);
    float4 waterTexCol = waterTex.Sample(sampler0, input.tex);
    
    float4 lightCol = calculateLighting(-lightDirection, input.normal, diffuseColour);
    
    // Input world position height will always be in the range of 0 - max amplitude set on CPU side
    float height = input.worldPos.y;
    
    // Check style of noise, this is required for the height based texturing to work
    // as ridged noise is inverted.
    if (noiseStyle == 1)
    {
        // Do ridged height based texturing
        return doRidgedHeightTexturing(height, snowTexCol, grassTexCol, waterTexCol, lightCol);
        
        // For Debug
        //return float4(input.normal, 1.0f);
    }
    
    // Do normal height based texturing for both normal noise and terraced noise
    return doNormalHeightTexturing(height, snowTexCol, grassTexCol, waterTexCol, lightCol);
    
    // For Debug
    //return float4(input.normal, 1.0f);
}


