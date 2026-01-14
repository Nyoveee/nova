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

    NormalizedFloat transparency;
}

// Vertex shader..
Vert{    
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. 
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz;
    vsOut.fragViewPos = vec3(view * worldSpace.position);

    vsOut.normal = normalize(worldSpace.normal);
}   

// Fragment shader..
Frag{
    return vec4(intensity * color, transparency); // ok
}