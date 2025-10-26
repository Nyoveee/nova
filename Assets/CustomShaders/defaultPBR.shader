Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
}
// Uniforms
Properties{
    sampler2D albedoMap;
    Color albedoColor;
    NormalizedFloat roughness;
    NormalizedFloat metallic; 
    NormalizedFloat occulusion;
}

Frag{
    vec4 albedoColor = texture(albedoMap, fsIn.textureUnit);
    vec3 pbrColor = PBRCaculation(vec3(albedoColor), fsIn.normal, roughness, metallic, occulusion);    
    FragColor = vec4(pbrColor, 1.0); // asd 5 * 2 - 9
}