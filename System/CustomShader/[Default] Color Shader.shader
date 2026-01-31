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
    return vec4(finalColor.rgb, transparency * sampledColor.a); // ok
}