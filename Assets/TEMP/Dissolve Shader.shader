
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Disable;
}

// Properties for material instances to configure..
Properties{
    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;

    sampler2D mainTexture;
    sampler2D dissolveTexture;
    
    NormalizedFloat dissolveThreshold;
    NormalizedFloat edgeWidth;

    Color edgeColor;
}

// Vertex shader..
Vert{
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. //s
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;
    vsOut.normal = worldSpace.normal;
    vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);
}

// Fragment shader..
Frag{
    vec4 albedo = texture(mainTexture, fsIn.textureUnit);
    float dissolve = texture(dissolveTexture, fsIn.textureUnit).r;

    vec3 pbrColor = PBRCaculation(vec3(albedo), fsIn.normal, roughness, metallic, occulusion);

    if (dissolve > dissolveThreshold)
    {
        // Discard or make transparent
        discard; 
    }
    else if (dissolve > dissolveThreshold - edgeWidth)
    {
        // Burning edge effect
        return vec4(edgeColor, 1.0);
    }
    else {
        return vec4(pbrColor, 1.0);
    }
}