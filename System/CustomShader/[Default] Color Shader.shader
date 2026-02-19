// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedo;
    Color color;
    float intensity;

    NormalizedFloat transparency;
    AlphaMap alphaMap;
    
    vec2 UVTiling; 
    vec2 UVOffset; 
}

// Vertex shader..
Vert{    
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace();
    gl_Position = calculateClipPosition(worldSpace.position);
    passDataToFragment(worldSpace);     // Pass attributes to fragment shader.. 
}   

// Fragment shader..
Frag{
    vec2 uv = UVTileAndOffset(fsIn.textureUnit, UVTiling, UVOffset);

    vec4 sampledColor = texture(albedo, uv);
    vec3 finalColor = sampledColor.rgb * color * intensity;

    float resultingAlpha = transparency * sampledColor.a;
    
    if(toUseAlphaMap) {
        resultingAlpha *= texture(alphaMap, uv).r;
    }

    return vec4(finalColor.rgb, resultingAlpha); // ok
}