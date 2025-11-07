// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color color;
}

// Vertex shader..
Vert{    
    gl_Position = calculateClipPosition(position);

    // pass texture units to fragment shader..
    vsOut.textureUnit = textureUnit; 
}

// Fragment shader..
Frag{
    FragColor = vec4(color, 1.0); // ok
}