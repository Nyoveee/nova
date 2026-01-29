
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

    NormalizedFloat roughness;
    NormalizedFloat metallic;
    NormalizedFloat occulusion;
}

// Vertex shader..
Vert{
    // Calculate world space of our local attributes..
    WorldSpace worldSpace = calculateWorldSpace();
    gl_Position = calculateClipPosition(worldSpace.position);
    passDataToFragment(worldSpace);     // Pass attributes to fragment shader.. 
}

// Fragment shader..
Frag{
    // Reference thebookofshaders.com/12/
    vec2 flowDirectionNorm = normalize(flowDirection);
    vec2 st = fsIn.textureUnit - timeElapsed * flowSpeed * flowDirectionNorm;

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
    vec3 color = mix(baseColor* emissivemultiplier, edgeColor,m_dist); // Closer to the point, the whiter it is(edge)
    vec3 pbrColor = PBRCaculation(color, fsIn.normal, roughness, metallic, occulusion);
    return vec4(pbrColor, 1.0);
}