// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class OscillatingMaterialField : Script
{
    public List<float> minValue;
    public List<float> maxValue;

    public List<int> materialIndex;
    public List<String> materialFieldName;
    
    public float speedMultiplier = 1f;
    public bool isActive = true;

    private MeshRenderer_ meshRenderer;
    private float timeElapsed = 0f;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(!isActive)
        {
            return;
        }

        timeElapsed += Time.V_DeltaTime();
        timeElapsed = timeElapsed % (Mathf.Deg2Rad * 360f);

        float interval = (Mathf.Sin(timeElapsed - Mathf.Deg2Rad * 90f) + 1f) / 2f;

        for(int i = 0; i < materialIndex.Count; ++i)
        {
            meshRenderer.setMaterialFloat(materialIndex[i], materialFieldName[i], Mathf.Interpolate(minValue[i], maxValue[i], interval, 1f));
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}