Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
}
// Uniforms
Properties{
    sampler2D albedoMap;
    vec3 albedoColor;
    float roughness;
    float metallic; 
    float occulusion;
}

Frag{
    vec4 albedoColor = texture(albedoMap, fsIn.textureUnit.rgb, 1.0);
    vec3 pbrColor = PBRCaculation(albedoColor, fsIn.normal, roughness, metallic, occulusion);    
    FragColor = vec4(pbrColor,1.0);
}