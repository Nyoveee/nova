
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
// ======================================================
// Uncomment this section of the code if you want to use the Color pipeline.
    gl_Position = calculateClipPosition(position);
    vsOut.textureUnit = textureUnit; 
}

// Fragment shader..
Frag{
	FragColor = vec4(color, 1.0);
}