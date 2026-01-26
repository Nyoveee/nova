
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

    Color colorMultiplier;
    Color colorAddition;
    float speedMultiplier;
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
    vec4 textureColor = texture(albedoMap, fsIn.textureUnit);

    float sineValue = (sin(timeElapsed * speedMultiplier));

    float baseAlpha = 0.05;
    float alphaVariance = 0.05;

    float alpha = baseAlpha + alphaVariance * sineValue;

    vec3 colorVariance = vec3(0.2);
    // vec3 finalColor = colorTint;
    vec3 finalColor = vec3(textureColor) * colorMultiplier + colorAddition;

	return vec4(finalColor, alpha);
}