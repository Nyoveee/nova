// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class StartBoatQuest : Quest
{
    [SerializableField]
    private List<string> weaverVoiceoverTexts;
    [SerializableField]
    private List<float> weaverVoiceoverTimes;
    [SerializableField]
    private Audio weaverVoiceoverAudio;

    [SerializableField]
    private GoddessBehaviour goddessBehaviour;
    [SerializableField]
    private string goddessVoiceoverText;
    [SerializableField]
    private float goddessVoiceoverTime;
    [SerializableField]
    private Audio goddessVoiceoverAudio;
    [SerializableField]
    private float goddessVoiceoverBeginTime;
    [SerializableField]
    private GameObject objectToPoint;
    [SerializableField]
    private float objectPointTime;
    [SerializableField]
    private Switch boatSwitch;
    [SerializableField]
    private BoatAudio boatAudio;

    private VoiceoverScript voiceoverScript;
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
    }
    public override void UpdateQuest()
    {
        if (boatSwitch.isSwitchActivated())
        {
            SetQuestState(QuestState.Success);
        }
    }
    public override void OnSuccess()
    {
        boatAudio.BeginBoatAudio();
        Invoke(() =>
        {
            voiceoverScript.TriggerVoiceOverSequence("Weaver", weaverVoiceoverTexts, weaverVoiceoverAudio, weaverVoiceoverTimes, false);
            Invoke(() =>
            {
                voiceoverScript.TriggerVoiceOver("Goddess", goddessVoiceoverText, goddessVoiceoverAudio, goddessVoiceoverTime, false);
                goddessBehaviour.PointAt(objectToPoint, objectPointTime);
            }, goddessVoiceoverBeginTime);
        }, 1);
       
    }
}