
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
    gl_Position = calculateClipPosition(position);
    vsOut.textureUnit = textureUnit; 
// ======================================================
}

// Fragment shader..
Frag{
    float sineValue = sin(timeElapsed * 5); 
	return vec4(colorTint * sineValue, 1.0);
}