
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

    float threshold;
    float thresholdGap;
    float emissiveMultiplier;

    int numOfLines;
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
    vec2 depthUV = gl_FragCoord.xy / screenDimensions; 

    float sceneDepth = texture(depthTexture, depthUV).r;
    sceneDepth = linearizeDepth(sceneDepth);
    float bubbleDepth = linearizeDepth(gl_FragCoord.z);

    float difference = abs(sceneDepth - bubbleDepth);

    if(difference < (threshold + thresholdGap) * numOfLines) {
        difference = mod(difference, threshold + thresholdGap);
    }

    float alpha = difference < threshold ? 1 : 0;

    float interval = clamp(difference / threshold, 0, 1);
    interval = smoothstep(0, 1, pow(1 - interval, 5));
	return vec4(mix(colorOne, colorTwo, interval) * emissiveMultiplier, interval);
}