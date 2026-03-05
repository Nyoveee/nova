
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    bool isActive;

    Color baseColor;
    Color emissiveColor;
    float emissiveMultiplier;
    float fresnelPower;
    float speedMultiplier;

    float resultingAlpha;

    sampler2D albedoMap;
    ORMMap packedMap;
    NormalMap normalMap;

    float lerpPercentage;
}

// Vertex shader..
Vert{
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace();

    gl_Position = calculateClipPosition(worldSpace.position);
    passDataToFragment(worldSpace);     // Pass attributes to fragment shader.. 
}

// Fragment shader..
// The general gist of a fresnel VFX is to utilize the fresnel factor + emissive color to create the glow on the outeredges..

// https://docs.unity3d.com/Packages/com.unity.shadergraph@6.9/manual/Fresnel-Effect-Node.html
// ^ same as schlink fresnel..

Frag{
    // === Handling normal ===
    vec3 _normal = getNormalFromMap(normalMap, fsIn.textureUnit);
    // vec3 _normal = normalize(fsIn.normal);

    // We calculate fresnel factor..
    vec3 viewDir = normalize(cameraPosition - fsIn.fragWorldPos);
    float NdotV = max(dot(_normal, viewDir), 0.0);
    float fresnelFactor = pow(1.0 - NdotV, fresnelPower);

    vec4 fresnelColor = vec4(emissiveColor * emissiveMultiplier, fresnelFactor);

    // ==================
    // PBR..
    // === Handling the 3 properties ===
    float roughness; 
    float metallic; 
    float occulusion;

    vec2 uv = fsIn.textureUnit;

    vec3 map = texture(packedMap, uv).rgb;
    metallic   = 1 - map.r;
    roughness  = 1 - map.g;
    occulusion = 1 - map.b;

    // ==================
    vec4 albedo = texture(albedoMap, uv);
    float pbrAlpha = albedo.a;

    vec3 pbrColor = PBRCaculation(vec3(albedo), _normal, roughness, metallic, occulusion);

	return mix(fresnelColor + vec4(baseColor, resultingAlpha), vec4(pbrColor, albedo.a), lerpPercentage);
}