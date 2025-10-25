
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
}

// Properties for material instances to configure..
Properties{
    Color colorTint1;
    Color colorTint2;
}

// Fragment shader..
Frag{
    // FragColor = vec4(colorTint2, 1.0); // ok
    if(gl_FragCoord.x > 1920 / 2) {
        FragColor = vec4(colorTint1, 1.0); // ok
    }
    else {
        FragColor = vec4(colorTint2, 1.0); // ok
    }
}