
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color colorTint1;
    Color colorTint2;

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;
}

Vert{    
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);
    gl_Position = calculateClipPosition(worldSpace.position);

    // Pass attributes to fragment shader.. //
    vsOut.textureUnit = textureUnit;
    vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;
    vsOut.normal = worldSpace.normal;
}

// Fragment shader..
Frag{
    vec3 color;

    if(gl_FragCoord.x > 1920 / 2) {
        color = colorTint1;
    }
    else {
        color = colorTint2;
    }

    vec3 pbrColor = PBRCaculation(color, fsIn.normal, roughness, metallic, occulusion);
    return vec4(pbrColor, 1.0);
}