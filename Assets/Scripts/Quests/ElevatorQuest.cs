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
    private GameObject elevatorLamp;

    [SerializableField]
    private Material elevatorLampActive;

    [SerializableField]
    private Vector3 activeLightColor;

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

    public override void OnEnter()
    {
        elevator.OpenTutorialDoor();

        if (elevatorLamp != null) {
            elevatorLamp.getComponent<MeshRenderer_>().changeMaterial(0, elevatorLampActive);

            foreach(GameObject child in elevatorLamp.GetChildren())
            {
                Light_ light = child.getComponent<Light_>();

                if(light != null)
                {
                    light.color = activeLightColor;
                }
            }
        }
    }

    public override void OnSuccess()
    {
        Destroy(checkPointIndicator);
        elevator.CloseTutorialDoor();

        Invoke(() =>
        {
            gameUIManager.ActivateDialogue(speaker, dialogues, timings, finalDialogueTime);
        }, delayForDialogue);
    }


}