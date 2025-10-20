ShaderName{
    Tags{
        Blending:AlphaBlending
    }
    Properties{
        glm::mat4 model;
    }
    Input{
        Position
        TextureUnit
    }
    Interface{
      textureUnit
    }
    Vert{    
        gl_Position = model * Position;
        textureUnit = TextureUnit;
    }
    Frag{
        FragColor = Color;
    }
}