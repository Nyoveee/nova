
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
    // vec4 worldPos = model * localScale * vec4(position, 1);
    // vec3 offsetWorldPos = vec3(worldPos) + normal;
    // worldPos = vec4(offsetWorldPos, 1);

    // // vec4 homoPos = vec4(localPos, 1);
    // gl_Position = projection * view * worldPos;
    gl_Position = calculateClipPosition(position + normal * 2);
    vsOut.textureUnit = textureUnit; 
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