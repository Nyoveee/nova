
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : NoDepthWriteTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color color;
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
	return vec4(color, 1.0);
}