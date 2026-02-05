// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class ElevatorQuest : Quest
{
    [SerializableField]
    private GameObject checkPointIndicator;
    [SerializableField]
    private Elevator elevator;
    [SerializableField]
    private GameObject missionObjectiveContainer;
    [SerializableField]
    private GameObject questInformationContainer;
    [SerializableField]
    private Vector3 newMissionObjectiveUILocation;
    [SerializableField]
    private Vector3 newQuestInformationUILocation;
    [SerializableField]
    private Audio elevatorSpeechAudio;
    [SerializableField]
    private string speaker;
    [SerializableField]
    private List<string> dialogues;
    [SerializableField]
    private List<float> timings;
    [SerializableField]
    private float finalDialogueTime;
    [SerializableField]
    private float delayForDialogue = 3;

    private AudioComponent_ audioComponent;
    private GameUIManager gameUIManager;

    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
    }

    public override void OnSuccess()
    {
        Destroy(checkPointIndicator);
        elevator.CloseTutorialDoor();

        missionObjectiveContainer.transform.position = newMissionObjectiveUILocation;
        questInformationContainer.transform.position = newQuestInformationUILocation;

        Invoke(() =>
        {
            audioComponent.PlaySound(elevatorSpeechAudio);
            gameUIManager.ActivateDialogue(speaker, dialogues, timings, finalDialogueTime);
        }, delayForDialogue);
    }

    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }


}