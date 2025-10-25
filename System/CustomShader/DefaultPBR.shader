{
    "blendingConfig": "AlphaBlending",
    "depthTestingMethod": "DepthTest",
    "fShaderCode": "\n    vec4 albedo = texture(albedoMap, fsIn.textureUnit);\n    vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, fsIn.normal, roughness, metallic, occulusion);    \n    FragColor = vec4(pbrColor, 1.0); // asd 5 * 2 - 9\n",
    "pipeline": "PBR",
    "uniforms": {
        "albedoMap": "sampler2D",
        "colorTint": "Color",
        "metallic": "NormalizedFloat",
        "occulusion": "NormalizedFloat",
        "roughness": "NormalizedFloat"
    }
}
