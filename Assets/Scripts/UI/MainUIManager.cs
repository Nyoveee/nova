// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

struct LightInfo
{
    public LightInfo(Light_ light, float initialIntensity)
    {
        this.light = light;
        this.initialIntensity = initialIntensity;
    }

    public Light_ light;
    public float initialIntensity;
}

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
    [SerializableField] GameObject particleEmitter;
    [SerializableField] GameObject playerLights;

    [SerializableField] public float initialLerpDuration = 2f;
    [SerializableField] public float lerpPower = 0.3f;

    [SerializableField] public float delay = 1f;
    [SerializableField] public float fadeDuration = 0.7f;
    [SerializableField] public float travelTime = 1f;

    [SerializableField] public float initialFadeOut = 1f;
    [SerializableField] public GameObject hubLights;

    [SerializableField] private AudioComponent_ mainMenuBGM;
    [SerializableField] private AudioComponent_ levelSelectBGM;
    [SerializableField] private float audioFadeDuration = 2f;

    [SerializableField] private Audio mainMenuAudio;
    [SerializableField] private Audio levelSelectAudio;

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
    private bool isLightTurningOn = false;
    private float fadeOutTimeElasped = 0f;

    private Callback fadeOutCallback;

    private List<LightInfo> playerLightsRuntime = new List<LightInfo>();

    private bool isFadingAudio = false;
    private float audioFadeTimeElapsed = 0f;

    private AudioComponent_ audioComponentToStartPlaying;
    private AudioComponent_ audioComponentToStopPlaying;
    private Audio audioToStartPlaying;
    private Audio audioToStopPlaying;

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

        darkOverlay.alpha = 1f;

        Invoke(() =>
        {
            canvasToFade = mainMenuUi;
            initialAlpha = 0f;
            finalAlpha = 1f;
            isFading = true;
            isLightTurningOn = true;

            fadeOutTimeElasped = 0f;
            particleEmitter.SetActive(true);

            fadeOutCallback = () => 
            {
                Invoke(() =>
                {
                    mainMenuUi.isInteractable = true;
                }, 1f); 
            };
            
        }, initialLerpDuration);

        Invoke(() =>
        {
            canvasToFade = darkOverlay;
            initialAlpha = 1f;
            finalAlpha = 0f;

            isFading = true;
            isCameraMoving = true;
        }, 1f);

        foreach (GameObject child in playerLights.GetChildren())
        {
            Light_ light = child.getComponent<Light_>();

            if (light != null) {
                playerLightsRuntime.Add(new LightInfo(light, light.intensity));
                light.intensity = 0f;
            }
        }
    }

    protected override void update()
    {
        FadingBGM();

        if (isCameraMoving)
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

    private void FadingBGM()
    {
        if(!isFadingAudio)
        {
            return;
        }

        if (audioFadeTimeElapsed > audioFadeDuration)
        {
            isFadingAudio = false;
            audioComponentToStartPlaying.volume = 1f;
        }

        if (audioFadeTimeElapsed < audioFadeDuration / 2f)
        {
            float interval = audioFadeTimeElapsed / (audioFadeDuration / 2f);
            audioComponentToStopPlaying.SetVolume(audioToStopPlaying, (1f - interval));
        }
        else
        {
            audioComponentToStopPlaying.volume = 0f;
            float interval = (audioFadeTimeElapsed - (audioFadeDuration / 2f)) / (audioFadeDuration / 2f);
            audioComponentToStartPlaying.volume = interval;
            audioComponentToStartPlaying.SetVolume(audioToStartPlaying, interval);
        }

        audioFadeTimeElapsed += Time.V_DeltaTime();
    }

    private void handleCameraMovement()
    {
        timeElapsed += Time.V_DeltaTime();

        float interval = timeElapsed / initialLerpDuration;

        float newY = Mathf.SmoothLerp(initialCameraPosition.y, finalCameraPosition.y, Mathf.Pow(interval, lerpPower));
        camera.transform.position = new Vector3(initialCameraPosition.x, newY, initialCameraPosition.z);

        if (timeElapsed > initialLerpDuration)
        {
            isCameraMoving = false;
        }
    }

    int counter = 0;
    bool isOn = true;

    private void RecursiveLightSwitch()
    {
        Invoke(() =>
        {
            isOn = !isOn;
            counter++;

            hubLights.SetActive(isOn);

            if (counter < 4)
            {
                RecursiveLightSwitch();
            }

        }, Random.Range(0.05f, 0.1f) * counter);
    }

    private void handleFading()
    {
        fadeOutTimeElasped += Time.V_DeltaTime();

        float interval = fadeOutTimeElasped / fadeDuration;

        canvasToFade.alpha = Mathf.Interpolate(initialAlpha, finalAlpha, interval, 1f);

        if (isLightTurningOn)
        {
            foreach (LightInfo lightInfo in playerLightsRuntime)
            {
                lightInfo.light.intensity = Mathf.Interpolate(0, lightInfo.initialIntensity, interval, 1f);
            }
        }

        if (fadeOutTimeElasped > fadeDuration)
        {
            isFading = false;
            isLightTurningOn = false;

            if (fadeOutCallback != null)
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

        audioComponentToStopPlaying = mainMenuBGM;
        audioFadeTimeElapsed = 0f;
        isFadingAudio = true;

        audioComponentToStartPlaying = levelSelectBGM;
        audioToStartPlaying = levelSelectAudio;
        audioToStopPlaying = mainMenuAudio;
        audioComponentToStopPlaying.SetVolume(levelSelectAudio, 0f);
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

        audioComponentToStopPlaying = levelSelectBGM;
        audioFadeTimeElapsed = 0f;
        isFadingAudio = true;

        audioComponentToStartPlaying = mainMenuBGM;
        audioToStartPlaying = mainMenuAudio;
        audioToStopPlaying = levelSelectAudio;
        audioComponentToStopPlaying.SetVolume(mainMenuAudio, 0f);
    }
}