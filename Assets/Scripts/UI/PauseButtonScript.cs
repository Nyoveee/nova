// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class PauseButtonScript : Script
{
    public Scene mainMenuScene;
    private GameUIManager uiManager;
    private AudioComponent_ audioComponent;

    [SerializableField]
    private Audio onHoverSFX;
    [SerializableField]
    private Audio onClickSFX;

    protected override void init()
    {
        uiManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        audioComponent = getComponent<AudioComponent_>();
    }

    public void ResumeGame()
    {
        uiManager.PauseHandler();
    }

    public void QuitGame()
    {
        Systems.Quit();
    }

    public void MainMenu()
    {
        SceneAPI.ChangeScene(mainMenuScene);
    }

    public void onHover()
    {
        audioComponent.PlaySound(onHoverSFX);
    }

    public void onPressed()
    { 
        audioComponent.PlaySound(onClickSFX);
    }

    public void RestartGame()
    {
        Systems.Restart();
    }
}