
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    NormalMap normalMap;

    Color emissiveColor;
    float emissiveMultiplier;
    float speedMultiplier;
    float fresnelPower;

    vec3 pulsatingDirection;
    
    float lineWidth;
    float lineSpacing;

    NormalizedFloat colorLerpPower;
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
    vec2 uv = fsIn.textureUnit;

    // === Handling normal ===
    vec3 _normal;
    _normal = getNormalFromMap(normalMap, uv); 

    // We calculate fresnel factor..
    vec3 viewDir = normalize(cameraPosition - fsIn.fragWorldPos);
    float NdotV = max(dot(_normal, viewDir), 0.0);
    float fresnelFactor = pow(1.0 - NdotV, fresnelPower);

    // timer..
    float sineOscillatingFactor = ((sin(timeElapsed * speedMultiplier)) + 1) / 2;

    // calculate pulsating direction..
    vec3 uvw = abs(fsIn.boundingBoxUVW - 0.5) * 2;
    float align = 1 - dot(uvw, normalize(pulsatingDirection));

    align += timeElapsed * speedMultiplier;
    // align = fract(align);

    align = (sin(align * lineWidth) + 1) / 2;
    align = pow(align, lineSpacing);

    // return vec4(vec3(align), 1);
    // vec3 color = mix(emissiveColor, emissiveColorTwo, pow(align, colorLerpPower));
    vec4 fresnelColor = vec4(emissiveColor * emissiveMultiplier, fresnelFactor * align);
	return fresnelColor;
}