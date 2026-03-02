// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class UnlockGateQuest : Quest
{
    [SerializableField]
    private Door vaultDoor;

    [SerializableField]
    private Door turbineToHubDoor;

    [SerializableField]
    private Switch hubSwitch;
    
    [SerializableField]
    private List<Light_> shadowCasters = new List<Light_>();

    [SerializableField]
    private GameObject switchSpotlight;

    [SerializableField]
    private float questCompletionDelay;

    [SerializableField]
    private float lightOffduration = 0.3f;

    private float timeElapsed = 0f;

    private float initialIntensity = 0f;

    private bool hasSucceeded = false;
    private bool isAnimating = false;

    // This function is first invoked when game starts.
    protected override void update()
    {
        if(isAnimating)
        {
            timeElapsed += Time.V_DeltaTime();
            timeElapsed = Mathf.Min(timeElapsed, lightOffduration);

            float interval = timeElapsed / lightOffduration;

            foreach (Light_ light in shadowCasters)
            {
                light.intensity = Mathf.Interpolate(initialIntensity, 0f, interval, 1f);
            }

            if (timeElapsed == lightOffduration) {
                isAnimating = false;
            }
        }
    }

    public override void OnEnter()
    {
        Invoke(() =>
        {
            if(shadowCasters.Count != 0)
            {
                initialIntensity = shadowCasters[0].intensity;
            }

            turbineToHubDoor.UnlockDoor();
            hubSwitch.enableSwitch();
            switchSpotlight.SetActive(true);
        }, 0f);
    }
   
    public override void OnSuccess()
    {
        vaultDoor.UnlockDoor();
        vaultDoor.OpenDoor();

        isAnimating = true;
    }
    public override void OnSkip()
    {
        vaultDoor.UnlockDoor();
        vaultDoor.OpenDoor();
        hubSwitch.forceEnableSwitch();

        isAnimating = true;
    }
    public override void UpdateQuest()
    {
        if(!hasSucceeded && hubSwitch.isSwitchActivated())
        {
            hasSucceeded = true;

            Invoke(() =>
            {
                SetQuestState(QuestState.Success);
            }, questCompletionDelay);
        }    
    }
}