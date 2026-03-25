
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
    AlphaMap alphaMap;
    NormalizedFloat alphaCutout;
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

    vec2 uv = fsIn.textureUnit;

    vec4 albedo = texture(albedoMap, uv);
    float resultingAlpha = albedo.a;
    
    if(toUseAlphaMap) {
        resultingAlpha *= texture(alphaMap, uv).r;
    }

    if (resultingAlpha < alphaCutout) {
        discard;
        return vec4(0);
    }

    vec3 map = texture(packedMap, uv).rgb;
    _metallic   = 1 - map.r;
    _roughness  = 1 - map.g;
    _occulusion = 1 - map.b;

    // === Handling normal ===
    vec3 _normal;
    if(toUseNormalMap) {
        _normal = getNormalFromMap(normalMap, uv); 
    }
    else {
        _normal = normalize(fsIn.normal);
    }

    vec3 pbrColor = PBRCaculation(vec3(albedo), _normal, _roughness, _metallic, _occulusion);

    return vec4(pbrColor, 1);
}