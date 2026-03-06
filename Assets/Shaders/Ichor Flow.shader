
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedoMap;
    ORMMap packedMap;
    NormalMap normalMap;

    float colorMultiplier;
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
    // === Handling the 3 properties ===
    vec2 uv = UVTileAndOffset(fsIn.textureUnit, vec2(1, 1), UVOffset);

    vec3 map = texture(packedMap, uv).rgb;
    float metallic   = map.r;
    float roughness  = map.g;
    float occulusion = map.b;

    // === Handling normal ===
    vec3 normal = getNormalFromMap(normalMap, uv); ;
    
    vec4 albedo = texture(albedoMap, uv);
    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorMultiplier, fsIn.normal, roughness, metallic, occulusion);
    return vec4(pbrColor, albedo.a);
}