// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GameGlobalReferenceManager : Script
{
    public GameObject[] vantagePoints;
    [SerializableField]
    private MeshRenderer_ levelRenderer;
    // This function is first invoked when game starts.
    protected override void init()
    {
        vantagePoints = GameObject.FindGameObjectsWithTag("Vantage Point");
        levelRenderer.setMaterialFloat(6, "intensity", 0.5f);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}