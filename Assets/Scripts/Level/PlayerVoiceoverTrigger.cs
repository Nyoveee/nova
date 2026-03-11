// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class PlayerVoiceoverTrigger : Script
{
    [SerializableField]
    private string speaker;
    [SerializableField]
    private string voiceOverText;
    [SerializableField]
    private float voiceOverTime;
    [SerializableField]
    private Audio voiceOverAudio;

    private VoiceoverScript voiceoverScript;
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
    }
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player")
        {
            voiceoverScript.TriggerVoiceOver(speaker, voiceOverText, voiceOverAudio, voiceOverTime, false);
            Destroy(gameObject);
        }
    }
}