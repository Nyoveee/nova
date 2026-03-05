// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Reflection.Metadata.Ecma335;

class GameplayStatisticsManager : Script
{
    private enum Level
    {
        IronHeights = 0,
        Acheron = 1,
        WeaverLair = 2
    }
    [SerializableField]
    private Level currentLevel;
    private float gameplayTime;
    protected override void init()
    {
        string levelDurationKey = "LevelCompletionDuration" + ((int)currentLevel).ToString();
        float lastTimeCompleted = PlayerPrefs.GetFloat(levelDurationKey);
        // This is just recently initialized, set to max
        if (lastTimeCompleted == 0)
            PlayerPrefs.SetFloat(levelDurationKey, float.MaxValue);
    }

    // This function is invoked every update.
    protected override void update()
    {
        gameplayTime += Time.V_DeltaTime();
    }

    public void CompleteLevel()
    {
        int completedLevels = PlayerPrefs.GetInt("CompletedLevel");
        // Completed Levels
        if (completedLevels == (int)currentLevel)
            PlayerPrefs.SetInt("CompletedLevel", completedLevels + 1);
        // Time Completed
        string levelDurationKey = "LevelCompletionDuration" + ((int)currentLevel).ToString();
        float lastTimeCompleted = PlayerPrefs.GetFloat(levelDurationKey);
        if (gameplayTime < lastTimeCompleted)
            PlayerPrefs.SetFloat(levelDurationKey, gameplayTime);

    }

}