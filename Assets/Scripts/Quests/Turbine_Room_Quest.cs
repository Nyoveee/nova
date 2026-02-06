// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Turbine_Room_Quest : Quest
{
    [SerializableField]
    private List<Switch> switches = new List<Switch>();

    [SerializableField]
    private Door turbineExitDoor;

    [SerializableField]
    private float questCompleteDelay = 1f;

    bool numOfActivatedSwitches;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {
          
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public override void UpdateQuest()
    {
        foreach(Switch switchObj in switches)
        {
            if(!switchObj.isSwitchActivated())
            {
                return;
            }
        }

        // at this point all switches are turned on.
        Invoke(() =>
        {
            SetQuestState(QuestState.Success);
            turbineExitDoor.UnlockDoor();

        }, questCompleteDelay);
    }

}