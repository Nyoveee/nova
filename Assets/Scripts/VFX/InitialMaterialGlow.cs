// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class InitialMaterialGlow : Script
{
    public List<float> minValue;

    public List<int> materialIndex;
    public List<String> materialFieldName;

    public float glowDuration = 2f;

    private MeshRenderer_ meshRenderer;
    private OscillatingMaterialField oscillatingMaterialField;

    private float timeElapsed = 0f;

    private bool isGlowingOne = false;
    private bool isGlowingTwo = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    { }

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
        oscillatingMaterialField = getScript<OscillatingMaterialField>();

        for (int i = 0; i < materialIndex.Count; ++i)
        {
            meshRenderer.setMaterialFloat(materialIndex[i], materialFieldName[i], 0f);
        }
        
        RendererAPI.iblMultiplier = 0.3f;

        Invoke(() =>
        {
            isGlowingOne = true;
        }, 1f);
    }

    // This function is invoked every update.
    protected override void update()
    {
        AnimateGlowOne();
        AnimateGlowTwo();
    }

    private void AnimateGlowTwo()
    {
        if (!isGlowingTwo)
        {
            return;
        }

        float interval = timeElapsed / glowDuration;
        RendererAPI.iblMultiplier = Mathf.Interpolate(0.3f, 1f, interval, 1f);

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > glowDuration)
        {
            RendererAPI.iblMultiplier = 1f;
            isGlowingOne = false;
        }
    }

    private void AnimateGlowOne()
    {
        if (!isGlowingOne)
        {
            return;
        }

        float interval = timeElapsed / glowDuration;

        for (int i = 0; i < materialIndex.Count; ++i)
        {
            meshRenderer.setMaterialFloat(materialIndex[i], materialFieldName[i], Mathf.Interpolate(0f, minValue[i], interval, 1f));
        }

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > glowDuration)
        {
            timeElapsed = 0f;
            isGlowingOne = false;
            isGlowingTwo = true;
            oscillatingMaterialField.isActive = true;
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    { }

    // This function is invoked when destroyed.
    protected override void exit()
    { }

    int counter = 0;
    bool isOn = true;
}