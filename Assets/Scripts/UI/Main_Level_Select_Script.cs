// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Main_Level_Select_Script : Script
{
    [SerializableField] private Image_ levelInfo;
    [SerializableField] private Text_ levelCompletedDurationText;
    [SerializableField] private List<Texture> levelInfoTextures;
    [SerializableField] private List<Button_> levelSelectUIs;
    [SerializableField] private List<Scene> levels;
    [SerializableField] ColorAlpha selectedLevelColor;

    [SerializableField] private Image_ darkOverlay;
    [SerializableField] private float sceneFadeOutDuration = 1.5f;

    int selectedLevel = 0;
    bool transitioning = false;
    float timeElapsed = 0;
    
    protected override void init()
    {
        SelectNewLevel(0);
    }

public void PreviewLevel(int level)
    {
        float levelCompleteDuration = PlayerPrefs.GetFloat("LevelCompletionDuration" + level, 0f);

        if (levelCompleteDuration > 0) 
        {
            int timeInSeconds = (int)levelCompleteDuration;
            int minutes = timeInSeconds / 60;
            int remainingSeconds = timeInSeconds % 60;

            levelCompletedDurationText.SetText(String.Format("{0:00} : {1:00}", minutes, remainingSeconds));
        }
        else
        {
            levelCompletedDurationText.SetText("-- : --");
        }

        levelInfo.SetTexture(levelInfoTextures[level]);
    }

    public void RestoreSelectedLevelPreview()
    {
        PreviewLevel(selectedLevel);
    }

    public void SelectNewLevel(int level)
    {
        selectedLevel = level;

        for (int i = 0; i < levelSelectUIs.Count; i++) {
            if(i == level)
            {
                levelSelectUIs[i].normalColor = selectedLevelColor;
            }
            else
            {
                levelSelectUIs[i].normalColor = new ColorAlpha(1, 1, 1, 1);
            }

            levelSelectUIs[i].forceColorUpdate();
        }
    }

    public void TransitionToLevel()
    {
        if(transitioning)
        {
            return;
        }

        darkOverlay.gameObject.SetActive(true);
        transitioning = true;
    }

    protected override void update()
    {
        if (!transitioning)
        {
            return;
        }

        timeElapsed += Time.V_DeltaTime();

        float interval = Mathf.Clamp(timeElapsed / sceneFadeOutDuration, 0f, 1f);
        darkOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, interval);

        if(interval == 1f)
        {
            SceneAPI.ChangeScene(levels[selectedLevel]);
        }
    }
}