// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class VoiceoverScript : Script
{
    [SerializableField]
    private Canvas_ dialogueUI;

    [SerializableField]
    private Text_ voiceOverSpeakerUI;

    [SerializableField]
    private Text_ voiceOverTextUI;
    
    private AudioComponent_ audioComponent;

    private PlayerWeaponController playerWeaponController;

    private Delegate Callback;

    private int totalRunningVoicelines;
    private List<bool> voiceLineActiveStates = new List<bool>();

    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        playerWeaponController = GameObject.FindWithTag("Player")?.getScript<PlayerWeaponController>();
    }
    protected override void update()
    {
        if (voiceLineActiveStates.Count == 0)
            return;
        bool b_HideUI = true;
        foreach(bool voiceLineActiveState in voiceLineActiveStates)
            if (voiceLineActiveState)
                b_HideUI = false;
        if (b_HideUI)
        {
            totalRunningVoicelines = 0;
            voiceLineActiveStates.Clear();
            dialogueUI.alpha = 0f;
        }
           
    }
    public void TriggerVoiceOver(string speaker, string text, Audio audio, float voiceOverTime, bool shouldDisableWeapon)
    {
        dialogueUI.gameObject.SetActive(true);
        audioComponent.PlaySound(audio);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();

        // Set Text
        voiceOverSpeakerUI?.SetText(speaker);
        voiceOverTextUI?.SetText(text);

        // Set Active State
        int currentVoicelineIndex = totalRunningVoicelines++;
        voiceLineActiveStates.Add(true);

        // Callback
        dialogueUI.alpha = 1f;
        Invoke(() =>
        {
            if (shouldDisableWeapon)
                playerWeaponController.EnableShooting();
            voiceLineActiveStates[currentVoicelineIndex] = false;
        }, voiceOverTime);
    }

    public void TriggerVoiceOverSequence(string speaker, List<string> text, Audio audio, List<float> voiceOverTimes, bool shouldDisableWeapon)
    {
        dialogueUI.gameObject.SetActive(true);
        audioComponent.PlaySound(audio);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();

        // Set Text
        voiceOverSpeakerUI?.SetText(speaker);

        // Set Text Sequence
        int index = -1;
        Callback callbackRecursive = null;

        // Set Active State
        int currentVoicelineIndex = totalRunningVoicelines++;
        voiceLineActiveStates.Add(true);

        // Callback
        Callback callback = () => {
            if (index == text.Count - 1)
            {
                if (shouldDisableWeapon)
                    playerWeaponController.EnableShooting();
                voiceLineActiveStates[currentVoicelineIndex] = false;
                return;
            }
            dialogueUI.alpha = 1f;
            voiceOverTextUI?.SetText(text[++index]);
            callbackRecursive();
        };
        
        callbackRecursive = () => { 
            Invoke(() => { callback(); }, voiceOverTimes[index]); 
        };

        callback();
    }
}