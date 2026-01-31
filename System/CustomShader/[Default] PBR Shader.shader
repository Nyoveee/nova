// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedoMap;
    sampler2D packedMap;
    sampler2D normalMap;
    sampler2D emissiveMap;

    bool toUsePackedMap;
    bool toUseNormalMap;
    bool toUseEmissiveMap;

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;

    Color colorTint;

    float emissiveStrength;

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
    // === Handling the 3 properties ===
    float _roughness; 
    float _metallic; 
    float _occulusion;

    vec2 uv = UVTileAndOffset(fsIn.textureUnit, UVTiling, UVOffset);

    if (toUsePackedMap) {
        vec3 map = texture(packedMap, uv).rgb;
        _metallic   = map.r;
        _roughness  = map.g;
        _occulusion = map.b;
    }     
    else {
        _roughness  = roughness;
        _metallic   = metallic;
        _occulusion = occulusion;
    }

    // === Handling normal ===
    vec3 _normal;
    if(toUseNormalMap) {
        // We assume that our normal map is compressed into BC5.
        // Since BC5 only stores 2 channels, we need to calculate z in runtime.
        vec2 bc5Channels = vec2(texture(normalMap, uv));
        
        // We shift the range from [0, 1] to  [-1, 1]
        bc5Channels = bc5Channels * 2.0 - 1.0; 

        // We calculate the z portion of the normal..
        vec3 sampledNormal = vec3(bc5Channels, sqrt(max(0.0, 1.0 - dot(bc5Channels.xy, bc5Channels.xy))));
        _normal = normalize(fsIn.TBN * sampledNormal);
    }
    else {
        _normal = normalize(fsIn.normal);
    }

    vec3 emissiveColor = vec3(0);

    if(toUseEmissiveMap) {
        emissiveColor = emissiveStrength * vec3(texture(emissiveMap, uv));
    }

    vec4 albedo = texture(albedoMap, uv);
    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, _normal, _roughness, _metallic, _occulusion);

    return vec4(emissiveColor + pbrColor, albedo.a);
}