
    // Specify tags for rendering..
    Tags{
        Blending : AlphaBlending;
        DepthTestingMethod : DepthTest;
        Culling : Enable;
    }

    // Properties for material instances to configure..
    Properties{
        sampler2D albedoMap;
        Color colorTint;

        NormalizedFloat roughness;
        NormalizedFloat metallic;
        NormalizedFloat occulusion;

        float outlineThickness;
    }

    // Vertex shader..
    Vert{
        gl_Position = calculateClipPosition(position);
    }

    // Fragment shader..
    Frag{
        vec4 outlineColor = vec4(colorTint.rgb, 1.0);
        outlineColor.rgb *= 1.5;
        FragColor = outlineColor;
        return FragColor;
    }