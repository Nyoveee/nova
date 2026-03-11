// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class BargeSurvivalQuest : Quest
{
    [SerializableField]
    private CannonWaveManager cannonWaveManager;
    [SerializableField]
    private GoddessBehaviour goddessBehaviour;
    [SerializableField]
    private EndOfLevel2 endOfLevel2;

    [SerializableField]
    private string goddessVoiceoverText;
    [SerializableField]
    private float goddessVoiceoverTime;
    [SerializableField]
    private Audio goddessVoiceoverAudio;

    private VoiceoverScript voiceoverScript;
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
    }
    protected override void update()
    {
        if (cannonWaveManager.IsWaveActive() && goddessBehaviour.gameObject != null && !goddessBehaviour.IsDisappearing())
            goddessBehaviour.BeginDisappearing();
        if (endOfLevel2.IsEndOfLevel())
        {
            SetQuestState(QuestState.Success);
        }
    }
    public override void OnSuccess()
    {
        voiceoverScript.TriggerVoiceOver("Goddess", goddessVoiceoverText, goddessVoiceoverAudio, goddessVoiceoverTime, false);
    }
}