Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
}

// Uniforms
Properties{
    Color color;
}

Frag{
    FragColor = vec4(color, 1.0);
}