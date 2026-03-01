// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Level_Select_UI_Script : Script
{
    private Main_Level_Select_Script levelSelectScript;
    private AudioComponent_ audioComponent;
    private Button_ button;

    private float levelCompleteDuration = 0;

    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField] private Audio onHoverSFX;
    [SerializableField] private Audio onClickSFX;

    [SerializableField] private int level = 1;

    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        button = getComponent<Button_>();

        int completedLevels = PlayerPrefs.GetInt("CompletedLevel", 0);
          
        if (completedLevels < level)
        {
            button.isInteractable = false;
        }
        else
        {
            button.isInteractable = true;
        }

        levelSelectScript = GameObject.FindWithTag("Main_Level_Select_Script")?.getScript<Main_Level_Select_Script>();
    }

    public void onHover()
    {
        audioComponent.PlaySound(onHoverSFX);
        levelSelectScript?.PreviewLevel(level);
    }

    public void onHoverLeave()
    { 
        levelSelectScript?.RestoreSelectedLevelPreview();
    }

    public void onPressed()
    {
    }

    public void onReleased()
    {
        audioComponent.PlaySound(onClickSFX);
        levelSelectScript?.SelectNewLevel(level);
    }
}