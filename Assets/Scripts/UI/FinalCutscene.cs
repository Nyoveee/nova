// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.UI.Input.Inking.Analysis;

class FinalCutscene : Script
{
    public Image_ blackOverlay;
    public List<GameObject> cutsceneShots;
    public List<CameraComponent_> cutsceneCameras;
    public List<float> cutsceneDuration;
    public List<float> cutsceneFadeDuration;
    public List<float> cutsceneFadeStayIn;
    public Audio finalCutsceneSfx;

    public Scene creditScene;

    bool isFading = false;
    bool hasFadedIn = false;
    float timeElapsed = 0f;

    Callback fadeToBlackCallback;
    Callback finishedFadingCallback;
    int currentCutsceneIndex = 0;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 1f);

        Invoke(() =>
        {
            // We skip to fade out phase..
            timeElapsed = cutsceneFadeDuration[0] + cutsceneFadeStayIn[0];
            hasFadedIn = true;
            isFading = true;
            fadeToBlackCallback = () => { };
            finishedFadingCallback = () => { };

            SetupCutsceneArea();

            getComponent<AudioComponent_>().PlaySound(finalCutsceneSfx);
        }, 1f);
    }

    // This function is invoked every update.
    protected override void update()
    {
        HandleFadeSequence();
    }

    public void HandleFadeSequence()
    {
        if (!isFading)
        {
            return;
        }

        // We begin fading to black.. 
        if (timeElapsed < cutsceneFadeDuration[currentCutsceneIndex])
        {
            float interval = timeElapsed / cutsceneFadeDuration[currentCutsceneIndex];
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, Mathf.Interpolate(0f, 1f, interval, 1f));
        }
        else if (timeElapsed < (cutsceneFadeDuration[currentCutsceneIndex] + cutsceneFadeStayIn[currentCutsceneIndex]) && !hasFadedIn)
        {
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 1f);
            hasFadedIn = true;
            fadeToBlackCallback();
        }
        // We begin fading from black..
        else if (timeElapsed > (cutsceneFadeDuration[currentCutsceneIndex] + cutsceneFadeStayIn[currentCutsceneIndex]))
        {
            float timeElapsedInRespect = timeElapsed - (cutsceneFadeDuration[currentCutsceneIndex] + cutsceneFadeStayIn[currentCutsceneIndex]);
            float interval = Mathf.Min(timeElapsedInRespect / cutsceneFadeDuration[currentCutsceneIndex], 1f);

            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, Mathf.Interpolate(1f, 0f, interval, 1f));
        }

        // Finished fading..
        if (timeElapsed > (2 * cutsceneFadeDuration[currentCutsceneIndex] + cutsceneFadeStayIn[currentCutsceneIndex]))
        {
            finishedFadingCallback();
            isFading = false;
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 0f);
        }

        timeElapsed += Time.V_DeltaTime();
    }
    
    public void BeginFadeSequence(Callback p_fadeToBlackCallback, Callback p_finishedFadingCallback)
    {
        timeElapsed = 0f;
        isFading = true;
        hasFadedIn = false;
        fadeToBlackCallback = p_fadeToBlackCallback;
        finishedFadingCallback = p_finishedFadingCallback;
    }

    public void SetupCutsceneArea()
    {
        if (currentCutsceneIndex != 0)
        {
            cutsceneCameras[currentCutsceneIndex - 1].camStatus = false;
        }

        cutsceneCameras[currentCutsceneIndex].camStatus = true;
        cutsceneCameras[currentCutsceneIndex].gameObject.getComponent<Sequence_>().play();
        cutsceneShots[currentCutsceneIndex]?.SetActive(true);

        ++currentCutsceneIndex;

        // move on to the next cutscene..
        if (currentCutsceneIndex != cutsceneShots.Count)
        {
            Invoke(() =>
            {
                BeginFadeSequence(
                    () =>
                    {
                        cutsceneShots[currentCutsceneIndex - 1]?.SetActive(false);
                        SetupCutsceneArea();
                    },
                    () =>
                    {

                    });
            }, cutsceneDuration[currentCutsceneIndex - 1]);
        }
        else
        {
            Invoke(() =>
            {
                BeginFadeSequence(
                    () =>
                    {
                        cutsceneShots[currentCutsceneIndex - 1]?.SetActive(false);
                        isFading = false; // dont fade out..

                        Invoke(() =>
                        {
                            SceneAPI.ChangeScene(creditScene);
                        }, 3f);
                    },
                    () =>
                    {

                    });
            }, cutsceneDuration[currentCutsceneIndex - 1]);
        }
    }
}