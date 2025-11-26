// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class ButtonScript : Script
{
    public Scene nextScene;

    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {}

    public void onHover()
    {
        Debug.Log("Hover");
    }

    public void onPressed()
    {
        Debug.Log("Pressed");

    }

    public void onReleased()
    {
        Debug.Log("Released");

        SceneAPI.ChangeScene(nextScene);
    }
}