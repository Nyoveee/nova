// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    sampler2D albedo;
    Color color;
    float intensity;

    NormalizedFloat transparency;
}

// Vertex shader..
Vert{    
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. 
    passDataToFragment(worldSpace);
}   

// Fragment shader..
Frag{
    vec3 finalColor = texture(albedo, fsIn.textureUnit).rgb * color * intensity;
    return vec4(finalColor , transparency); // ok
}