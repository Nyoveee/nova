// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ShootTutorialQuest : Quest
{
    [SerializableField]
    private GameObject grunt;

    [SerializableField]
    private Switch switchGameObject;

    [SerializableField]
    private float questCompleteDelay = 3f;

    [SerializableField]
    private float tutorialDelayTime;
    private bool succeeded = false;
    private bool ichorTutorialShown = false;
    private bool executeTutorialShown = false;
    private GameUIManager gameUIManager;
    protected override void init()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
    }
    public override void UpdateQuest() {
        
        if(!ichorTutorialShown && grunt.getScript<Grunt>().WasRecentlyDamaged())
        {
            ichorTutorialShown = true;
            Invoke(() =>
            {
                gameUIManager.ToggleTutorial();
            }, tutorialDelayTime);
        }
        if(!executeTutorialShown && grunt.getScript<Grunt>().IsExecutable())
        {
            executeTutorialShown = true;
            Invoke(() =>
            {
                gameUIManager.ToggleTutorial();
            }, tutorialDelayTime);
        }
        if (!succeeded && grunt != null && grunt.getScript<Grunt>().IsDead())
        {
            succeeded = true;

            Invoke(() =>
            {
                SetQuestState(QuestState.Success);
                switchGameObject?.enableSwitch();

            }, questCompleteDelay);
        }
    }

}