
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
    // vec2 depthUV = gl_FragCoord.xy / screenDimensions; 

    // float sceneDepth = texture(depthTexture, depthUV).r;
    // sceneDepth = linearizeDepth(sceneDepth);
    // float bubbleDepth = linearizeDepth(gl_FragCoord.z);

    // float difference = abs(sceneDepth - bubbleDepth);
    // float alpha = difference < threshold ? 1 : 0;

    // float interval = pow(1 - clamp(difference / threshold, 0, 1), 5);
    // vec4 intersectionColor = vec4(mix(colorOne, colorTwo, interval) * emissiveMultiplier, interval);

    float alpha = pow(clamp((1 - fsIn.boundingBoxUVW.y * yUVMultiplier), 0, 1), power);
    vec4 mainColor = vec4(mix(colorOne, colorTwo, alpha) * emissiveMultiplier, alpha);

	return mainColor;
}