// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class BossAudio : Script
{
    [SerializableField]
    public List<Audio> walkSoundsAudio;

    [SerializableField]
    public int footStepIndex = 0;

    [SerializableField]
    public List<Audio> fireRocketAudio;

    [SerializableField]
    public List<Audio> shockWaveAudio;

    [SerializableField]
    public List<Audio> meleeAttackAudio;

    [SerializableField]
    public List<Audio> bossHitAudio;

    [SerializableField]
    public Audio bossRageAudio;

    [SerializableField]
    private string weaverVoiceoverText;
    [SerializableField]
    private float weaverVoiceoverTime;
    [SerializableField]
    private Audio weaverVoiceoverAudio;
    [SerializableField]
    private float weaverVoiceoverDelay;

    private VoiceoverScript voiceoverScript;
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
    }
    public void TriggerDeathVoiceOver()
    {
        Invoke(() =>
        {
            voiceoverScript.TriggerVoiceOver("Weaver", weaverVoiceoverText, weaverVoiceoverAudio, weaverVoiceoverTime, false);
        }, weaverVoiceoverDelay);   
    }

}