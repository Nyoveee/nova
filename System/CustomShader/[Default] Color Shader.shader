// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color color;
    float intensity;
}

// Vertex shader..
Vert{    
    gl_Position = calculateClipPosition(position);

    // pass texture units to fragment shader..
    vsOut.textureUnit = textureUnit; 
}

// Fragment shader..
Frag{
    return vec4(intensity * color, 1.0); // ok
}