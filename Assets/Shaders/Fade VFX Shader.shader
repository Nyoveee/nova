
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color color;
    float emissiveMultiplier;
    vec3 direction;
    float alphaPower;

    float alphaMultiplier;
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
    float alpha = pow(fsIn.textureUnit.x, alphaPower);
	return vec4(color * emissiveMultiplier, alpha * alphaMultiplier);
}