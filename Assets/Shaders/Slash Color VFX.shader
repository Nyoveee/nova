
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D image;
    vec2 UVOffset;

    Color colorMultiplier;
    float intensity;
    float power;
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
    vec2 uv = UVTileAndOffset(fsIn.textureUnit, vec2(1, 1), UVOffset);
    vec4 color = texture(image, uv);
	return vec4(color.rgb * colorMultiplier * intensity, pow(color.a * alpha, power));
}