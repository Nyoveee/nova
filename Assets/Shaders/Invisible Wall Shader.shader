
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedoMap;
    Color colorOne;
    Color colorTwo;

    float power;
    float yUVMultiplier;
    float emissiveMultiplier;

    float alpha;
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
    float intensity = pow(clamp((1 - fsIn.boundingBoxUVW.y * yUVMultiplier), 0, 1), power);
    vec4 mainColor = vec4(mix(colorOne, colorTwo, intensity) * emissiveMultiplier, intensity * alpha);

	return mainColor;
}