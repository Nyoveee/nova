
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

    NormalizedFloat resultingAlpha;

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

    float time = timeElapsed * speedMultiplier;
    time = sin(time);

    float brightness = emissiveMultiplier + (emissiveMultiplier * 0.7 * time);

    // We calculate fresnel factor..
    vec3 viewDir = normalize(cameraPosition - fsIn.fragWorldPos);
    float NdotV = max(dot(_normal, viewDir), 0.0);
    float fresnelFactor = pow(1.0 - NdotV, fresnelPower);

    vec3 color = baseColor + emissiveColor * brightness * fresnelFactor;
	return vec4(color, resultingAlpha);
}