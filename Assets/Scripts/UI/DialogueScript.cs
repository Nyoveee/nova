using ScriptingAPI;
using Windows.Management.Deployment;

public delegate void Callback();

// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class DialogueScript : Script
{
    private Vector3 startPosition;
    private Vector3 endPosition;
    private float currentTransitionTime;
    private float finalDialogueTime;
    private List<string> dialogue;
    int currentIndex;

    private float timeElapsed = 0f;

    // ----- Fade sequence.. -----
    bool isFading = false;
    bool hasFadedIn = false;

    Callback fadeToBlackCallback;
    Callback finishedFadingCallback;
    // -------------------------- 

    private PlayerController_V2 playerBody;
    private CameraComponent_ playerCamera;
    private AudioComponent_ audioComponent_;

    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float fadeTransitionTime = 1.2f;

    [SerializableField]
    private float fadeStayTime = 0.4f;

    [SerializableField]
    private List<CameraComponent_> cutsceneCameras;

    [SerializableField]
    private List<String> cutsceneAreaTitle;

    [SerializableField]
    private List<String> cutsceneSubtitle;

    [SerializableField]
    private float sceneCutsceneDuration = 3;

    [SerializableField]
    private GameObject cutsceneUI;

    [SerializableField]
    private GameObject playerContainerUI;

    [SerializableField]
    private Text_ dialogueText;
    
    [SerializableField]
    private Text_ speakerText;

    [SerializableField]
    private Text_ sceneTitleText;

    [SerializableField]
    private Text_ sceneSubtitleText;

    [SerializableField]
    private Image_ blackOverlay;

    [SerializableField]
    private Audio elevatorSpeechAudio;

    [SerializableField]
    private GameObject missionObjectiveContainer;
    [SerializableField]
    private GameObject questInformationContainer;

    [SerializableField]
    private Vector3 newMissionObjectiveUILocation;
    [SerializableField]
    private Vector3 newQuestInformationUILocation;

    private enum DialogueState
    {
        StartTransition,
        InDialogue,
        EndTransition
    }
    private DialogueState dialogueState;

    protected override void init()
    {
        playerBody = GameObject.FindWithTag("Player")?.getScript<PlayerController_V2>();
        playerCamera = GameObject.FindWithTag("PlayerCamera")?.getComponent<CameraComponent_>();

        audioComponent_ = getComponent<AudioComponent_>();
    }

    protected override void update()
    {
        HandleFadeSequence();

#if false
        currentTransitionTime -= Time.V_DeltaTime();
        currentTransitionTime = Mathf.Max(currentTransitionTime, 0);
        if (dialogueState == DialogueState.StartTransition)
        {
            gameObject.transform.position = Vector3.Lerp(endPosition, startPosition, currentTransitionTime / dialogueBoxTransitionTime);
            if (currentTransitionTime <= 0)
                dialogueState = DialogueState.InDialogue;
        }
        if(dialogueState == DialogueState.InDialogue)
        {
            if (currentIndex != dialogue.Count)
                return;
            finalDialogueTime -= Time.V_DeltaTime();
            if(finalDialogueTime < 0)
            {
                currentTransitionTime = dialogueBoxTransitionTime;
                dialogueState = DialogueState.EndTransition;
                return;
            }
           
        }
        if(dialogueState == DialogueState.EndTransition)
        {
            gameObject.transform.position = Vector3.Lerp(startPosition, endPosition, currentTransitionTime / dialogueBoxTransitionTime);
            if(currentTransitionTime <= 0)
                gameObject.SetActive(false);
        }
#endif
    }

    private int currentCameraIndex = 0;

    public void SetupCutsceneArea()
    {
        if (currentCameraIndex != 0) {
            cutsceneCameras[currentCameraIndex - 1].camStatus = false;
        }
        
        cutsceneCameras[currentCameraIndex].camStatus = true;
        cutsceneCameras[currentCameraIndex].gameObject.getComponent<Sequence_>().play();

        sceneSubtitleText.SetText(cutsceneSubtitle[currentCameraIndex]);
        sceneTitleText.SetText(cutsceneAreaTitle[currentCameraIndex]);

        ++currentCameraIndex;

        // move on to the next cutscene..
        if (currentCameraIndex != cutsceneCameras.Count)
        {
            Invoke(() =>
            {
                BeginFadeSequence(
                    () =>
                    {
                        SetupCutsceneArea();
                    },
                    () =>
                    {
                        
                    });
            }, sceneCutsceneDuration);
        }
        // fade out back to player..
        else
        {
            Invoke(() =>
            {
                BeginFadeSequence(
                    () =>
                    {
                        // swap camera..
                        cutsceneCameras[currentCameraIndex - 1].camStatus = false;
                        playerCamera.camStatus = true;

                        // swap visible UI..
                        playerContainerUI.SetActive(true);
                        cutsceneUI.SetActive(false);

                        playerBody.movementIsEnabled = true;
                    },
                    () =>
                    {

                    });
            }, sceneCutsceneDuration);
        }
    }

    public void BeginDialogueSequence(string speaker, List<string> text, List<float> times, float finalDialogueTime)
    {
        BeginFadeSequence(
        () =>
        {
            // Reposition mission objective UI..
            missionObjectiveContainer.transform.position = newMissionObjectiveUILocation;
            questInformationContainer.transform.position = newQuestInformationUILocation;

            // swap visible UI..
            playerContainerUI.SetActive(false);
            cutsceneUI.SetActive(true);

            playerBody.movementIsEnabled = false;

            // swap camera..
            playerCamera.camStatus = false;

            // begin cutscene transition effect..
            SetupCutsceneArea();
        }, 
        () =>
        {
            // begin chaining of dialogue..
            audioComponent_.PlaySound(elevatorSpeechAudio);

            speakerText.SetText(speaker);
            currentIndex = 0;
            dialogue = text;

            // There's a chance if this function is called during another dialogue it might mess up due to invokes still existing
            // if that's the case can replace with a normal update
            for (int i = 0; i < dialogue.Count; ++i)
            {
                Invoke(() =>
                {
                    if (currentIndex == dialogue.Count - 1)
                        this.finalDialogueTime = finalDialogueTime;
                    dialogueText.SetText(dialogue[currentIndex++]);
                }, times[i]);
            }
        });
    }

    public void BeginFadeSequence(Callback p_fadeToBlackCallback, Callback p_finishedFadingCallback)
    {
        timeElapsed = 0f;
        isFading = true;
        hasFadedIn = false;
        fadeToBlackCallback = p_fadeToBlackCallback;
        finishedFadingCallback = p_finishedFadingCallback;
    }

    public void HandleFadeSequence()
    {
        if (!isFading)
        {
            return;
        }

        // We begin fading to black.. 
        if (timeElapsed < fadeTransitionTime)
        {
            float interval = timeElapsed / fadeTransitionTime;
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, Mathf.Interpolate(0f, 1f, interval, 1f));
        }
        else if (timeElapsed < (fadeTransitionTime + fadeStayTime) && !hasFadedIn)
        {
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 1f);
            hasFadedIn = true;
            fadeToBlackCallback();
        }
        // We begin fading from black..
        else if (timeElapsed > (fadeTransitionTime + fadeStayTime))
        {
            float timeElapsedInRespect = timeElapsed - (fadeTransitionTime + fadeStayTime);
            float interval = Mathf.Min(timeElapsedInRespect / fadeTransitionTime, 1f);

            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, Mathf.Interpolate(1f, 0f, interval, 1f));
        }

        // Finished fading..
        if (timeElapsed > (2 * fadeTransitionTime + fadeStayTime))
        {
            finishedFadingCallback();
            isFading = false;
            blackOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 0f);
        }

        timeElapsed += Time.V_DeltaTime();
    }
}