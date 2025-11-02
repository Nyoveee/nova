// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GameUIManager : Script
{
    [SerializableField]
    private Transform_? dashBar = null;

    private float dashBarMaxSize;
    protected override void init()
    {
        dashBarMaxSize = dashBar.scale.x;
    }

    public void SetDashBarProgress(float current, float max)
    {
        if (max == 0){
            Debug.LogError("Progress bar max cannot be zero");
            return;
        }
        Vector3 scale = dashBar.scale;
        scale.x = current / max * dashBarMaxSize;
        dashBar.scale = scale;

    }
}