
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
}

// Vertex shader..
Vert{
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
#if 0
    gl_Position = calculateClipPosition(position);
    vsOut.textureUnit = textureUnit; 
#endif

// ======================================================
// Comment this section of the code if you want to use the Color pipeline.
#if 1
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. //
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;
    vsOut.normal = worldSpace.normal;
    vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);
#endif
// ======================================================
}

// Fragment shader..
Frag{
    vec4 albedo = texture(albedoMap, fsIn.textureUnit);
    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, fsIn.normal, roughness, metallic, occulusion);
    FragColor = vec4(pbrColor, 1.0);
}