// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class FadeIn : Script
{
    public float delay = 1f;
    public float fadeInDuration = 2f;

    MeshRenderer_ meshRenderer;
    bool isFading = false;
    float timeElapsed = 0f;
    
    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();

        Invoke(() =>
        {
            isFading = true;
        }, delay);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(!isFading)
        {
            return;
        }

        meshRenderer.setMaterialFloat(0, "transparency", timeElapsed / fadeInDuration);

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > fadeInDuration) { 
            isFading = false;
            meshRenderer.setMaterialFloat(0, "transparency", 1f);
        }
    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}