
// Specify tags for rendering..
Tags{
    Blending : AlphaBlending;
    DepthTestingMethod : DepthTest;
    Culling : Enable;
}

// Properties for material instances to configure..
Properties{
    Color baseColor;
    Color edgeColor;
    float emissivemultiplier;
    vec2 gridsize;
    vec2 flowDirection;
    float flowSpeed;
    float waveSpeed;
    float waveAmplitude;
    float waveLength;
    int numberOfWaveSamples;

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;

    Color intersectionColor;
    float threshold;
    float foamScale;

    float noiseOffsetScale;
    NormalizedFloat alphaStepSize;
}

Functions{
    float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
    vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
    vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

    float noise(vec3 p){
        vec3 a = floor(p);
        vec3 d = p - a;
        d = d * d * (3.0 - 2.0 * d);

        vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
        vec4 k1 = perm(b.xyxy);
        vec4 k2 = perm(k1.xyxy + b.zzww);

        vec4 c = k2 + a.zzzz;
        vec4 k3 = perm(c);
        vec4 k4 = perm(c + 1.0);

        vec4 o1 = fract(k3 * (1.0 / 41.0));
        vec4 o2 = fract(k4 * (1.0 / 41.0));

        vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
        vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

        return o4.y * d.y + o4.x * (1.0 - d.y);
    }

    float rand(float n){return fract(sin(n) * 43758.5453123);}

    float noise(float p){
        float fl = floor(p);
        float fc = fract(p);
        return mix(rand(fl), rand(fl + 1.0), fc);
    }

    float rand(vec2 n) { 
        return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
    }

    float noise(vec2 p){
        vec2 ip = floor(p);
        vec2 u = fract(p);
        u = u*u*(3.0-2.0*u);
        
        float res = mix(
            mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
            mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
        return res*res;
    }
}

// Vertex shader..
Vert{
    WorldSpace worldSpace = calculateWorldSpace();

    vec2 flowDirectionNorm = normalize(flowDirection);
    vec2 direction = flowDirectionNorm;
    float phase = waveSpeed * (2.f/ waveLength);
    float frequency = 2/waveLength;
    float directionIter = 2; // In radians
    float amplitude = waveAmplitude;
    for(int i = 0; i < numberOfWaveSamples;++i){
        float x = dot(direction, worldSpace.position.xz) * frequency + timeElapsed * phase;
        worldSpace.position.y += amplitude * sin(x);
        direction = normalize(vec2(cos(directionIter*i),sin(directionIter*i)));
        amplitude *= 0.82;
        frequency *= 1.12;
    }
    
    gl_Position = calculateClipPosition(worldSpace.position);
    passDataToFragment(worldSpace);
}

// Fragment shader..
Frag{
    // Reference thebookofshaders.com/12/
    vec2 flowDirectionNorm = normalize(flowDirection);
    vec2 st = fsIn.textureUnit + timeElapsed * flowSpeed * flowDirectionNorm;

    // scale by gridSize
    st *= gridsize;

    // get the tile position and local position within the tile
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);

    float m_dist = 1;
    // Get the closer distance of the surrounding 9 grids with random point
    for(int y = -1; y<=1 ; ++y){
        for(int x = -1; x<=1;++x){
            // Neighbor in the grid
            vec2 neighbor = vec2(float(x),float(y));

            // Random position from current grid to neighbour grid
            vec2 p = i_st + neighbor;
            vec2 point = fract(sin(vec2(dot(p,vec2(127.1, 311.7)),dot(p,vec2(269.5,183.3))))* 43758.5453);

            // Set the point position
            point = 0.5 + 0.5 * sin(6.2831 * point);

            // Get the local distance between the point and the current pixel
            vec2 diff = neighbor + point - f_st;
            float dist = length(diff);
            
            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }

    vec3 color = mix(baseColor * emissivemultiplier, edgeColor * emissivemultiplier, m_dist); // Closer to the point, the whiter it is(edge)
    vec3 pbrColor = PBRCaculation(color, fsIn.normal, roughness, metallic, occulusion);

    vec2 depthUV = gl_FragCoord.xy / screenDimensions; 

    float sceneDepth = texture(depthTexture, depthUV).r;
    sceneDepth = linearizeDepth(sceneDepth);
    float bubbleDepth = linearizeDepth(gl_FragCoord.z);

    vec2 direction = flowDirectionNorm * waveSpeed * timeElapsed;

    float difference = abs(sceneDepth - bubbleDepth);

    float alpha = clamp(difference / threshold, 0, 1);

    float factor = 1 - alpha;
    
    alpha -= noise(fsIn.fragWorldPos.xz * foamScale + st) * noiseOffsetScale * factor;
    alpha = step(alpha, alphaStepSize);

    // alpha += step(noise(fsIn.fragWorldPos.xz * foamScale + st), 0.5);

    // float alpha = (difference + noise((fsIn.fragWorldPos.xz) * foamScale + st) * noiseOffsetScale) < threshold ? 1 : 0;
    // float alpha = difference < threshold ? 1 : 0;

    vec3 foamColor = intersectionColor * alpha;

    return vec4(pbrColor + foamColor, 1.0);
}