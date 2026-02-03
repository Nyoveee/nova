
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    bool isActive;

    Color emissiveColor;
    Color emissiveColorTwo;
    float emissiveMultiplier;
    float speedMultiplier;
    float fresnelPower;

    vec3 pulsatingDirection;
    
    float lineWidth;
    float lineSpacing;

    NormalizedFloat colorLerpPower;

    NormalMap normalMap;
}

// Vertex shader..
Vert{
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace();

    // worldSpace.position.xyz += worldSpace.normal * 0.05;

    gl_Position = calculateClipPosition(worldSpace.position);
    passDataToFragment(worldSpace);     // Pass attributes to fragment shader.. 
}

// Fragment shader..
// The general gist of a fresnel VFX is to utilize the fresnel factor + emissive color to create the glow on the outeredges..

// https://docs.unity3d.com/Packages/com.unity.shadergraph@6.9/manual/Fresnel-Effect-Node.html
// ^ same as schlink fresnel..

Frag{
    // switch..
    if(isActive == false) {
        return vec4(0);
    }

    // === Handling normal ===
    vec3 _normal;
    if(toUseNormalMap) {
        _normal = getNormalFromMap(normalMap, fsIn.textureUnit); 
    }
    else {
        _normal = normalize(fsIn.normal);
    }

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
    vec3 color = mix(emissiveColor, emissiveColorTwo, pow(align, colorLerpPower));
	return vec4(color * emissiveMultiplier, fresnelFactor * align);
}