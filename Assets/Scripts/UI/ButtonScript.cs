// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class ButtonScript : Script
{
    public Scene nextScene;
    /***********************************************************
    Components
    ***********************************************************/
    private AudioComponent_ audioComponent;

    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Audio onHoverSFX;
    [SerializableField]
    private Audio onClickSFX;
    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    { }

    public void onHover()
    {
        audioComponent.PlaySound(onHoverSFX);
    }

    public void onPressed()
    {
    }

    public void onReleased()
    {
        audioComponent.PlaySound(onClickSFX);
        SceneAPI.ChangeScene(nextScene);
    }

    public void Quit()
    {
        Systems.Quit();
    }

    public void Restart()
    {
        Systems.Restart();
    }

    // used by settings
    public void DisableSettingsUI()
    {
        MainSettingsScript setting = GameObject.FindWithTag("Setting UI")?.getScript<MainSettingsScript>();

        if (setting != null) {
            setting.toShowSettingsUI(false);
        }
    }

    public void EnableSettingsUI()
    {
        MainSettingsScript setting = GameObject.FindWithTag("Setting UI")?.getScript<MainSettingsScript>();

        if (setting != null)
        {
            setting.toShowSettingsUI(true);
        }
    }
}