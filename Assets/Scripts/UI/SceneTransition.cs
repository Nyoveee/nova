// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SceneTransition : Script
{
    private enum TransitionState
    {
        Nothing,
        FadeIn,
        FadeOut
    }
    [SerializableField]
    private Scene sceneToChange;
    [SerializableField]
    private float transitionTime;
    [SerializableField]
    private TransitionState transitionState;
    [SerializableField]
    private Image_ transitionImage;

    private float currentTransitionTime;
    private GameplayStatisticsManager gameplayStatisticsManager;
    protected override void init()
    {
        if(transitionState == TransitionState.FadeOut)
            currentTransitionTime = transitionTime;
        gameplayStatisticsManager = GameObject.FindWithTag("Gameplay Statistics Manager")?.getScript<GameplayStatisticsManager>();
    }
    // This function is invoked every update.
    protected override void update()
    {
        if(transitionState!= TransitionState.Nothing)
            currentTransitionTime -= Time.V_DeltaTime();
        switch (transitionState)
        {
            case TransitionState.FadeIn:
                {
                    float alpha = Mathf.SmoothLerp(1, 0, currentTransitionTime / transitionTime);
                    ColorAlpha imageColor = new ColorAlpha(0, 0, 0, alpha);
                    transitionImage.colorTint = imageColor;
                    if (currentTransitionTime <= 0)
                        SceneAPI.ChangeScene(sceneToChange);  
                    break;
                }
            case TransitionState.FadeOut:
                {
                    float alpha = Mathf.SmoothLerp(0, 1, currentTransitionTime / transitionTime);
                    ColorAlpha imageColor = new ColorAlpha(0, 0, 0, alpha);
                    transitionImage.colorTint = imageColor;
                    if (currentTransitionTime <= 0)
                        transitionState = TransitionState.Nothing;
                    break;
                }

        }
    }
    public void BeginTransition()
    {
        gameplayStatisticsManager?.CompleteLevel();
        transitionState = TransitionState.FadeIn;
        currentTransitionTime = transitionTime;
    }

}