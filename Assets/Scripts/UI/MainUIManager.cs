// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Runtime.CompilerServices;

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
    [SerializableField] Canvas_ darkOverlay;
    [SerializableField] Transform_ cameraMainMenuPos;

    [SerializableField] public float initialLerpDuration = 2f;
    [SerializableField] public float lerpPower = 0.3f;

    [SerializableField] public float delay = 1f;
    [SerializableField] public float fadeDuration = 0.7f;
    [SerializableField] public float travelTime = 1f;

    [SerializableField] public float initialFadeOut = 1f;
    [SerializableField] public GameObject hubLights;

    private CameraComponent_ cameraComponent;
    private Sequence_ cameraSequence;

    private bool isTransitioning = false;
    private bool isCameraMoving = false;
    private float timeElapsed = 0f;

    private CurrentUI state = CurrentUI.MainMenu;
    private Callback callback;

    private Canvas_ fromCanvas;
    private Canvas_ toCanvas;

    private Vector3 initialCameraPosition;
    private Vector3 finalCameraPosition;

    private Canvas_ canvasToFade;
    private float initialAlpha = 1f;
    private float finalAlpha = 0f;
    private bool isFading = false;
    private float fadeOutTimeElasped = 0f;

    private Callback fadeOutCallback;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        cameraComponent = camera.getComponent<CameraComponent_>();
        cameraSequence = camera.getComponent<Sequence_>();

        initialCameraPosition = camera.transform.position;
        finalCameraPosition = cameraMainMenuPos.position;

        Invoke(() =>
        {
            canvasToFade = mainMenuUi;
            initialAlpha = 0f;
            finalAlpha = 1f;
            isFading = true;

            fadeOutTimeElasped = 0f;

            fadeOutCallback = () => mainMenuUi.isInteractable = true;
        }, 4.2f);

        Invoke(() =>
        {
            hubLights.SetActive(true);
        }, 3f);

        Invoke(() =>
        {
            canvasToFade = darkOverlay;
            initialAlpha = 1f;
            finalAlpha = 0f;

            cameraSequence.play();
            isFading = true;
        }, 1f);
    }

    protected override void update()
    {
        if(isCameraMoving)
        {
            handleCameraMovement();
        }

        if(isFading)
        {
            handleFading();
        }

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
        else if (timeElapsed < (fadeDuration + delay))
        {
            fromCanvas.alpha = 0f;
        }
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
            toCanvas.alpha = 1f;
        }

        timeElapsed += Time.V_DeltaTime(); 
    }

    private void handleCameraMovement()
    {
        timeElapsed += Time.V_DeltaTime();

        float interval = timeElapsed / initialLerpDuration;

        camera.transform.position = Vector3.Lerp(initialCameraPosition, finalCameraPosition, Mathf.Pow(interval, lerpPower));

        if (timeElapsed > initialLerpDuration)
        {
            isCameraMoving = false;
        }
    }

    private void handleFading()
    {
        fadeOutTimeElasped += Time.V_DeltaTime();

        float interval = fadeOutTimeElasped / fadeDuration;

        canvasToFade.alpha = Mathf.Interpolate(initialAlpha, finalAlpha, interval, 1f);

        if (fadeOutTimeElasped > fadeDuration)
        {
            isFading = false;

            if(fadeOutCallback != null)
            {
                fadeOutCallback();
            }
        }
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