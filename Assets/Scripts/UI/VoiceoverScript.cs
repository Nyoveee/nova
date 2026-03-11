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
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        playerWeaponController = GameObject.FindWithTag("Player")?.getScript<PlayerWeaponController>();
    }

    public void TriggerVoiceOver(string speaker, string text, Audio audio, float voiceOverTime, bool shouldDisableWeapon)
    {
        audioComponent.PlaySound(audio);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();

        // Set Text
        voiceOverSpeakerUI?.SetText(speaker);
        voiceOverTextUI?.SetText(text);

        dialogueUI.alpha = 1f;
        // Trigger fade out once done 
        Invoke(() =>
        {
            if (shouldDisableWeapon)
                playerWeaponController.EnableShooting();
            dialogueUI.alpha = 0f;
        }, voiceOverTime);
    }

    public void TriggerVoiceOverSequence(string speaker, List<string> text, Audio audio, List<float> voiceOverTimes, bool shouldDisableWeapon)
    {
        audioComponent.PlaySound(audio);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();

        // Set Text
        voiceOverSpeakerUI?.SetText(speaker);


        // Set Text Sequence
        int index = -1;
        Callback callbackRecursive = null;
        
        Callback callback = () => {
            if (index == text.Count - 1)
            {
                if (shouldDisableWeapon)
                    playerWeaponController.EnableShooting();
                dialogueUI.alpha = 0f;
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