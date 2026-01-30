
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedoMap;
    Color colorTint;

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;
    float bloom;
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
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
#if 1    
	vec4 outlineColor = vec4(1.0 * bloom, 0.843 * bloom, 0.0 * bloom, 1.0);
    return outlineColor;
#endif

// ======================================================
// Comment this section of the code if you want to use the Color pipeline.
// #if 1
//     vec4 albedo = texture(albedoMap, fsIn.textureUnit);
//     vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, fsIn.normal, roughness, metallic, occulusion);
//     FragColor = vec4(pbrColor, 1.0);
// #endif
}