{
    "blendingConfig": "AlphaBlending",
    "cullingConfig": "Enable",
    "depthTestingMethod": "DepthTest",
    "fShaderCode": "\n// ======================================================\n// Uncomment this section of the code if you want to use the Color pipeline.\n#if 1    \n\tvec4 outlineColor = vec4(1.0 * bloom, 0.843 * bloom, 0.0 * bloom, 1.0);\n    return outlineColor;\n#endif\n\n// ======================================================\n// Comment this section of the code if you want to use the Color pipeline.\n// #if 1\n//     vec4 albedo = texture(albedoMap, fsIn.textureUnit);\n//     vec3 pbrColor = PBRCaculation(vec3(albedo) * colorTint, fsIn.normal, roughness, metallic, occulusion);\n//     FragColor = vec4(pbrColor, 1.0);\n// #endif\n",
    "pipeline": "Color",
    "uniforms": {
        "albedoMap": "sampler2D",
        "bloom": "float",
        "colorTint": "Color",
        "metallic": "NormalizedFloat",
        "occulusion": "NormalizedFloat",
        "roughness": "NormalizedFloat"
    },
    "vShaderCode": "\n// ======================================================\n// Uncomment this section of the code if you want to use the Color pipeline.\n#if 1\n    gl_Position = calculateClipPosition(position);\n#endif\n\n// ======================================================\n// Comment this section of the code if you want to use the Color pipeline.\n// #if 1\n//     // Calculate world space of our local attributes..\n//     WorldSpace worldSpace = calculateWorldSpace(position, normal, tangent);\n//     gl_Position = calculateClipPosition(worldSpace.position);\n\n//     // Pass attributes to fragment shader.. //\n//     vsOut.textureUnit = textureUnit;\n//     vsOut.fragWorldPos = worldSpace.position.xyz / worldSpace.position.w;\n//     vsOut.normal = worldSpace.normal;\n//     vsOut.TBN = calculateTBN(worldSpace.normal, worldSpace.tangent);\n// #endif\n// ======================================================\n"
}
