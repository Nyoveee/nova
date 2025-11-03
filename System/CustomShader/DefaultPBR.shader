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

    bool toUsePackedMap;
    sampler2D packedMap;

    bool toUseNormalMap;
    sampler2D normalMap;

    bool toUseEmissiveMap;
    sampler2D emissiveMap;
}

// Vertex shader..
Vert{
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. //
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;
    vsOut.normal = worldSpace.normal;
    vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);
}

// Fragment shader..
Frag{
    // === Handling the 3 properties ===
    float _roughness; 
    float _metallic; 
    float _occulusion;

    if (toUsePackedMap) {
        vec3 map = texture(packedMap, fsIn.textureUnit).rgb;
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
        vec2 bc5Channels = vec2(texture(normalMap, fsIn.textureUnit));
        
        // We shift the range from [0, 1] to  [-1, 1]
        bc5Channels = bc5Channels * 2.0 - 1.0; 

        // We calculate the z portion of the normal..
        vec3 sampledNormal = vec3(bc5Channels, 1 - bc5Channels.x * bc5Channels.x - bc5Channels.y * bc5Channels.y);
        _normal = normalize(fsIn.TBN * sampledNormal);
    }
    else {
        _normal = normalize(fsIn.normal);
    }

    vec3 emissiveColor = vec3(0);

    if(toUseEmissiveMap) {
        emissiveColor = vec3(texture(emissiveMap, fsIn.textureUnit));
    }

    vec4 albedo = texture(albedoMap, fsIn.textureUnit);
    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, _normal, _roughness, _metallic, _occulusion);

    FragColor = vec4(emissiveColor + pbrColor, 1.0);
}