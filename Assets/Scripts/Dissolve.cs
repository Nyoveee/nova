// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Dissolve : Script
{
    public float speedMultiplier;
    public Colour edgeColor;

    private MeshRenderer_ meshRenderer;
    private float timeElapsed = 0f;

    // This function is first invoked when game starts.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
        meshRenderer.setMaterialVector3(0, "edgeColor", new Vector3(edgeColor.r, edgeColor.g, edgeColor.b));
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        float interval = Mathf.Sin(timeElapsed);
        interval = (interval + 1f) / 2f;

        timeElapsed += Time.V_FixedDeltaTime() * speedMultiplier; 

        meshRenderer.setMaterialFloat(0, "dissolveThreshold", interval);
    }

}