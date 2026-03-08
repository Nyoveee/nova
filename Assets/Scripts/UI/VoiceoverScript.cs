// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class VoiceoverScript : Script
{
    [SerializableField]
    private GameObject cutSceneUI;
    [SerializableField]
    private GameObject blackBarUI;
    [SerializableField]
    private GameObject voiceOverSpeakerUI;
    [SerializableField]
    private GameObject voiceOverTextUI;
    [SerializableField]
    private GameObject sceneTextUI;
    [SerializableField]
    private Vector3 voiceOverSpeakerTempPosition;
    [SerializableField]
    private Vector3 voiceOverTextTempPosition;

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
        // audioComponent.PlaySound(audio);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();
        cutSceneUI.SetActive(true);
        blackBarUI.SetActive(false);
        sceneTextUI.SetActive(false);
        // Temperory
        voiceOverSpeakerUI.transform.position = voiceOverSpeakerTempPosition;
        voiceOverTextUI.transform.position = voiceOverTextTempPosition;
        // Set Text
        voiceOverSpeakerUI.getComponent<Text_>()?.SetText(speaker);
        voiceOverTextUI.getComponent<Text_>()?.SetText(text);

        // Trigger fade out once done 
        Invoke(() =>
        {
            cutSceneUI.SetActive(false);
            if (shouldDisableWeapon)
                playerWeaponController.EnableShooting();
        }, voiceOverTime);
    }
    public void TriggerVoiceOverSequence(string speaker, List<string> text, Audio audio, List<float> voiceOverTimes, bool shouldDisableWeapon)
    {
        // audioComponent.PlaySound(audio);
        cutSceneUI.SetActive(true);
        blackBarUI.SetActive(false);
        sceneTextUI.SetActive(false);
        if (shouldDisableWeapon)
            playerWeaponController.DisableShooting();
        // Temperory
        voiceOverSpeakerUI.transform.position = voiceOverSpeakerTempPosition;
        voiceOverTextUI.transform.position = voiceOverTextTempPosition;
        // Set Text
        voiceOverSpeakerUI.getComponent<Text_>()?.SetText(speaker);
        // Set Text Sequence
        int index = -1;
        Callback callbackRecursive = null;
        Callback callback = () => {
            if (index == text.Count - 1)
            {
                if (shouldDisableWeapon)
                    playerWeaponController.EnableShooting();
                cutSceneUI.SetActive(false);
                return;
            }
            voiceOverTextUI.getComponent<Text_>()?.SetText(text[++index]);
            callbackRecursive();
        };
        callbackRecursive = () => { 
            Invoke(() => { callback(); }, voiceOverTimes[index]); 
        };
        callback();
    }
}