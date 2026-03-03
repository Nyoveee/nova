// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.IO.IsolatedStorage;
using ScriptingAPI;

class GoddessAnimateGlow : Script
{
    [SerializableField] float glowDuration = 2f;
    [SerializableField] float delay = 2f;

    [SerializableField] List<Material> goddessMaterials;

    SkinnedMeshRenderer_ renderer;
    
    List<float> allMaterialAlpha = new List<float>();
    List<float> allMaterialEmissive = new List<float>();

    float timeElapsed = 0f;
    bool isGlowing = false;
    
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        renderer = getComponent<SkinnedMeshRenderer_>();

        for(int i = 0; i < goddessMaterials.Count; i++)
        {
            allMaterialAlpha.Add(renderer.getMaterialFloat(i, "resultingAlpha"));
            allMaterialEmissive.Add(renderer.getMaterialFloat(i, "emissiveMultiplier"));
        }
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(!isGlowing)
        {
            return;
        }

        float interval = timeElapsed / glowDuration;

        for (int i = 0; i < goddessMaterials.Count; i++)
        {
            renderer.setMaterialFloat(i, "resultingAlpha", Mathf.Interpolate(allMaterialAlpha[i], 1f, interval, 1f));
            renderer.setMaterialFloat(i, "lerpPercentage", interval);

            if(interval < 0.5f)
            {
                float emissiveInterval = interval * 2f;
                renderer.setMaterialFloat(i, "emissiveMultiplier", Mathf.Interpolate(allMaterialEmissive[i], 100f, emissiveInterval, 1f));
            }
            else
            {
                float emissiveInterval = (interval - 0.5f) * 2f;
                renderer.setMaterialFloat(i, "emissiveMultiplier", Mathf.Interpolate(100f, 1f, emissiveInterval, 1f));
            }
        }

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > glowDuration)
        {
            isGlowing = false;

            for (int i = 0; i < goddessMaterials.Count; i++)
            {
                renderer.changeMaterial(i, goddessMaterials[i]);
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void toGlow()
    {
        isGlowing = true;
        timeElapsed = 0f;
    }
}