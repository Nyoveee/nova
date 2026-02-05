// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class DeathButtonScript : Script
{
    public Scene mainMenuScene;
    public void RestartFromCheckpoint()
    {
        GameUIManager uiManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        uiManager.OnRestartButtonPressed();
    }

    public void MainMenu()
    {
        SceneAPI.ChangeScene(mainMenuScene);
    }
}