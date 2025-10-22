ShaderName {
    Tags {
        Blending:AlphaBlending
    }
    Properties {
        Texture image;
    }
    Input {
        Position
        TextureUnit
    }
    Interface {
        textureUnit
    }
    Vert {    
        gl_Position = model * Position;
        textureUnit = TextureUnit;
    }
    Frag {
        FragColor = texture(image, textureUnit);
    }
}