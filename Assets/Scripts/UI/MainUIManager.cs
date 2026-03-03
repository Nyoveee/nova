// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MainUIManager : Script
{
    private delegate void Callback();

    enum CurrentUI
    {
        MainMenu,
        LevelSelect
    };

    [SerializableField] GameObject camera;
    [SerializableField] Canvas_ mainMenuUi;
    [SerializableField] Canvas_ levelSelectUi;

    [SerializableField] public float delay = 1f;
    [SerializableField] public float fadeDuration = 0.7f;
    [SerializableField] public float travelTime = 1f;

    private CameraComponent_ cameraComponent;
    private Sequence_ cameraSequence;

    private bool isTransitioning = false;
    private float timeElapsed = 0f;

    private CurrentUI state = CurrentUI.MainMenu;
    private Callback callback;

    private Canvas_ fromCanvas;
    private Canvas_ toCanvas;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        cameraComponent = camera.getComponent<CameraComponent_>();
        cameraSequence = camera.getComponent<Sequence_>();
    }

    protected override void update()
    {
        if(!isTransitioning)
        {
            return;
        }

        // Initial fade out..
        if (timeElapsed < fadeDuration)
        {
            float interval = timeElapsed / fadeDuration;
            fromCanvas.alpha = Mathf.Interpolate(1f, 0f, interval, 1f);
        }
        // afk..
        // ..
        // Final fade in..
        else if (timeElapsed > (fadeDuration + delay) && timeElapsed < (2 * fadeDuration + delay)) { 
            float interval = (timeElapsed - (fadeDuration + delay)) / fadeDuration;
            toCanvas.alpha = Mathf.Interpolate(0f, 1f, interval, 1f);
        }
        // Finished faidng..
        else if (timeElapsed > (2 * fadeDuration + delay))
        {
            isTransitioning = false;
            toCanvas.isInteractable = true;
        }

        timeElapsed += Time.V_DeltaTime(); 
    }

    public void GoToLevelSelectUI()
    {
        if (state == CurrentUI.LevelSelect)
        {
            return;
        }

        state = CurrentUI.LevelSelect;
        timeElapsed = 0f;
        isTransitioning = true;

        cameraSequence.speedMultiplier = 1f;
        cameraSequence.play();

        mainMenuUi.isInteractable = false;

        fromCanvas = mainMenuUi;
        toCanvas = levelSelectUi;
    }

    public void GoToMainMenuUI()
    {
        if (state == CurrentUI.MainMenu)
        {
            return;
        }

        state = CurrentUI.MainMenu;
        timeElapsed = 0f;
        isTransitioning = true;


        cameraSequence.speedMultiplier = -1f;
        cameraSequence.play();

        levelSelectUi.isInteractable = false;

        fromCanvas = levelSelectUi;
        toCanvas = mainMenuUi;
    }
}