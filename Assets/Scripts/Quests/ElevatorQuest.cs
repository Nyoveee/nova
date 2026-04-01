// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Graphics.Display;

class ElevatorQuest : Quest
{
    [SerializableField]
    private GameObject checkPointIndicator;

    [SerializableField]
    GameObject playerOrientation;

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

    [SerializableField]
    GameObject goddessVaultRisingTrigger;

    private DialogueScript dialogueScript;

    private GameObject playerBody;
    private GameObject playerHead;

    protected override void init()
    {
        dialogueScript = GameObject.FindWithTag("Game UI Manager")?.getScript<DialogueScript>();
        playerBody = GameObject.FindWithTag("Player");
        playerHead = GameObject.FindWithTag("PlayerHead");
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
    public override void OnSkip()
    {
        goddessVaultRisingTrigger.SetActive(false);
        checkPointIndicator.SetActive(false);
    }
    public override void OnSuccess()
    {
        checkPointIndicator.SetActive(false);
        elevator.CloseTutorialDoor();
        Invoke(() =>
        {
            dialogueScript?.BeginDialogueSequence(speaker, dialogues, timings, finalDialogueTime);
        }, delayForDialogue);
        Invoke(() =>
        {
            Vector3 elevatorCenter = new Vector3(elevator.gameObject.transform.position.x, playerBody.transform.position.y, elevator.gameObject.transform.position.z);
            playerBody.transform.position = elevatorCenter;
            playerHead.transform.localRotation = Quaternion.LookRotation(elevator.gameObject.transform.front);
            playerOrientation.transform.localRotation = Quaternion.LookRotation(elevator.gameObject.transform.front);
        }, delayForDialogue + dialogueScript.GetFadeTransitionTime());
    }


}