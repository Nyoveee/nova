#version 450 core

layout (location = 1) out vec3 gNormal;

uniform bool toUseNormalMap;
uniform sampler2D normalMap;

in VS_OUT {
    vec3 normal;
    vec2 textureUnit;
    mat3 TBN;
} fsIn;

void main()
{             
#if 0
    vec3 _normal;

    if(toUseNormalMap) {
        // We assume that our normal map is compressed into BC5.
        // Since BC5 only stores 2 channels, we need to calculate z in runtime.
        vec2 bc5Channels = vec2(texture(normalMap, fsIn.textureUnit));
        
        // We shift the range from [0, 1] to  [-1, 1]
        bc5Channels = bc5Channels * 2.0 - 1.0; 

        // We calculate the z portion of the normal..
        vec3 sampledNormal = vec3(bc5Channels, sqrt(max(0.0, 1.0 - dot(bc5Channels.xy, bc5Channels.xy))));
        _normal = normalize(fsIn.TBN * sampledNormal);
    }
    else {
        _normal = normalize(fsIn.normal);
    }

    gNormal = _normal;
#else
    gNormal = fsIn.normal;
#endif
}  