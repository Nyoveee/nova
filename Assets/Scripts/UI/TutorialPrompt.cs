// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class TutorialPrompt : Script
{
    private enum TutorialState
    {
        Fadein,
        Waiting,
        Fadeout,
    }
    [SerializableField]
    private float fadeTime;
    [SerializableField]
    private GameObject tutorialImages;
    [SerializableField]
    private Image_ notedButtonImage;
    [SerializableField]
    private Button_ notedButton;
    
    private List<Image_> tutorials = new List<Image_>();
    private int currentIndex = -1;
    private float currentFadeTime;
    private GameUIManager gameUIManager;
    private TutorialState currentState = TutorialState.Fadein;
    protected override void init()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        notedButton.enable = false;
        foreach (GameObject tutorialImage in tutorialImages.GetChildren())
            tutorials.Add(tutorialImage.getComponent<Image_>());
    }
    protected override void update()
    {
        currentFadeTime += Time.V_DeltaTime();
        float alpha = 0;
        switch (currentState)
        {
            case TutorialState.Fadein:
                alpha = Mathf.Interpolate(0, 1, currentFadeTime / fadeTime, 1);
                alpha = Mathf.Min(alpha, 1f);
                tutorials[currentIndex].colorTint = notedButtonImage.colorTint =  new ColorAlpha(1f, 1f, 1f, alpha);
                if(alpha == 1f)
                {
                    currentState = TutorialState.Waiting;
                    notedButton.enable = true;
                }
                break;
            case TutorialState.Fadeout:
                alpha = Mathf.Interpolate(1, 0, currentFadeTime / fadeTime, 1);
                alpha = Mathf.Max(alpha, 0);
                tutorials[currentIndex].colorTint = notedButtonImage.colorTint = new ColorAlpha(1f, 1f, 1f, alpha);
                if (alpha == 0f)
                    gameUIManager.ToggleTutorial();
                break;
        }
    }
    public void BeginNextTutorial()
    {
        ++currentIndex;
        if (currentIndex == tutorials.Count)
        {
            Debug.LogWarning("No more tutorials to show");
            gameUIManager.ToggleTutorial();
            return;
        }
        currentState = TutorialState.Fadein;
        currentFadeTime = 0;

    }
    public void DisablePrompt()
    {
        currentState = TutorialState.Fadeout;
        notedButton.enable = false;
        currentFadeTime = 0;
    }
}