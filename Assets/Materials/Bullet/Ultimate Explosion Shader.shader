// Reference https://thebookofshaders.com/12/

// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D dissolveTexture;
    
    NormalizedFloat dissolveThreshold;
    NormalizedFloat edgeWidth;

    sampler2D mainTexture;
    Color edgeColor;

    float intensity;
}

// Vertex shader..
Vert{
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
    gl_Position = calculateClipPosition(position);
    vsOut.textureUnit = textureUnit; 
}

// Fragment shader..
Frag{
    vec3 mainColor = texture(mainTexture, fsIn.textureUnit).rgb * intensity; 
    float dissolve = texture(dissolveTexture, fsIn.textureUnit).r;

    if (dissolve > dissolveThreshold)
    {
        // Discard or make transparent
        discard; 
    }
    else if (dissolve > dissolveThreshold - edgeWidth)
    {
        // Burning edge effect
        return vec4(edgeColor * intensity, dissolveThreshold);
    }
    else {
        return vec4(mainColor, dissolveThreshold);
    }
}