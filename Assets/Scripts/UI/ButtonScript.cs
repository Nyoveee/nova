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
    private Canvas_ parentCanvas;

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
        audioComponent.PlaySound(onClickSFX);
    }

    public void onReleased()
    {
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
        ToShowPopUI("Setting UI", false, false);
    }

    public void EnableSettingsUI()
    {
        ToShowPopUI("Setting UI", true, false);
    }

    public void EnableSettingsUIWithFade()
    {
        ToShowPopUI("Setting UI", true, true);
    }

    public void DisableSettingsUIWithFade()
    {
        ToShowPopUI("Setting UI", false, true);
    }

    public void DisableControlsUI()
    {
        ToShowPopUI("Controls UI", false, false);
    }

    public void EnableControlsUI()
    {
        ToShowPopUI("Controls UI", true, false);
    }

    private void ToShowPopUI(string tagName, bool value, bool toFadeParentCanvas)
    {
        GameObject.FindWithTag(tagName).getScript<UIPopupScript>().toShowUI(value, parentCanvas, toFadeParentCanvas);
    }

    public void StartChapter()
    {
        GameObject.FindWithTag("Main_Level_Select_Script")?.getScript<Main_Level_Select_Script>()?.TransitionToLevel();
    }

    public void GoToLevelSelect()
    {
        GameObject.FindWithTag("Main UI Manager")?.getScript<MainUIManager>()?.GoToLevelSelectUI();
    }

    public void GoToMainMenu()
    {
        GameObject.FindWithTag("Main UI Manager")?.getScript<MainUIManager>()?.GoToMainMenuUI();
    }
}