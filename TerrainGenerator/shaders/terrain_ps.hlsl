// Terrain pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D snowTex : register(t0);
Texture2D grassTex : register(t1);
Texture2D sandTex : register(t2);
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

float4 doNormalHeightTexturing(float height, float4 snowTexCol, float4 grassTexCol, float4 sandTexCol, float4 lightCol)
{
    float4 blendTex = 0.0f;
    float lowerBound = 0.0f;        // a1
    float upperBound = 0.0f;        // a2
    float lowerLerpCoeff = 0.0;     // b1
    float upperLerpCoeff = 1.0f;    // b2
    float lerpCoeff = 0.0f;
   
    float heightNorm = normalize(height);
    
    // This is a max negative value to ensure that any height that has been displaced downwards
    // is textured with sand, otherwise we run the risk of not meeting the conditions of each of these
    // statements and therefore end up texturing with snow, the last return statement, which is of course
    // not what should be at the bottom of a valley
    lowerBound = -20.0f; // a1
    upperBound = 3.0f; // a2

    //                      a1    a2                      b1  b2  
    // Map boundary range [-0.2, 0.2] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // If at the boundary of this current tex and the next one, then blend between them
    if (height <= 0.0f || (height >= lowerBound && height <= upperBound))
    {
        blendTex = lerp(sandTexCol, grassTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    lowerBound = 3.0f; // a1
    upperBound = 7.0f; // a2
       
    //                      a1    a2                      b1  b2  
    // Map boundary range [-0.2, 0.2] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
        
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(grassTexCol, snowTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    return saturate(lightCol) * snowTexCol;
}

float4 doRidgedHeightTexturing(float height, float4 snowTexCol, float4 grassTexCol, float4 sandTexCol, float4 lightCol)
{
    float4 blendTex = 0.0f;
    float lowerBound = 0.0f; // a1
    float upperBound = 0.0f; // a2
    float lowerLerpCoeff = 0.0; // b1
    float upperLerpCoeff = 1.0f; // b2
    float lerpCoeff = 0.0f;
   
    float heightNorm = normalize(height);
    
    // This is a max negative value to ensure that any height that has been displaced downwards
    // is textured with sand, otherwise we run the risk of not meeting the conditions of each of these
    // statements and therefore end up texturing with snow, the last return statement, which is of course
    // not what should be at the bottom of a valley
    lowerBound = -15.0f; // a1
    upperBound = -5.0f; // a2

    //                      a1    a2                      b1  b2  
    // Map boundary range [-0.2, 0.2] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
    
    // If at the boundary of this current tex and the next one, then blend between them
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(sandTexCol, grassTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    lowerBound = -5.0f; // a1
    upperBound = -0.5f; // a2
       
    //                      a1    a2                      b1  b2  
    // Map boundary range [-0.2, 0.2] to lerp coeff range [0, 1]
    lerpCoeff = lowerLerpCoeff + (height - lowerBound) * (upperLerpCoeff - lowerLerpCoeff) / (upperBound - lowerBound);
        
    if (height >= lowerBound && height <= upperBound)
    {
        blendTex = lerp(grassTexCol, snowTexCol, lerpCoeff);
        return saturate(lightCol) * blendTex;
    }

    return saturate(lightCol) * snowTexCol;
}

float4 main(InputType input) : SV_TARGET
{
    float4 snowTexCol = snowTex.Sample(sampler0, input.tex);
    float4 grassTexCol = grassTex.Sample(sampler0, input.tex);
    float4 sandTexCol = sandTex.Sample(sampler0, input.tex);
    
    float4 lightCol = calculateLighting(-lightDirection, input.normal, diffuseColour);
    
    // Input world position height will always be in the range of 0 - max amplitude set on CPU side
    float height = input.worldPos.y;
    
    // Check style of noise, this is required for the height based texturing to work
    // as ridged noise is inverted.
    if (noiseStyle == 1)
    {
        // Do ridged height based texturing
        return doRidgedHeightTexturing(height, snowTexCol, grassTexCol, sandTexCol, lightCol);
    }
    
    // Do normal height based texturing for both normal noise and terraced noise
    return doNormalHeightTexturing(height, snowTexCol, grassTexCol, sandTexCol, lightCol);
}


