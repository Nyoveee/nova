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
    private Vector3 voiceOverSpeakerTempPosition;
    [SerializableField]
    private Vector3 voiceOverTextTempPosition;

    private AudioComponent_ audioComponent;

    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();   
    }
    public void TriggerVoiceOver(string speaker, string text, Audio audio, float voiceOverTime)
    {
        //audioComponent.PlaySound(audio);

        cutSceneUI.SetActive(true);
        blackBarUI.SetActive(false);
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
        }, voiceOverTime);

    }
}