// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class UnlockGateQuest : Quest
{
    [SerializableField]
    private Door vaultDoor;

    [SerializableField]
    private Door turbineToHubDoor;

    [SerializableField]
    private Switch hubSwitch;

    [SerializableField]
    private float questCompletionDelay;

    private bool hasSucceeded = false;

    // This function is first invoked when game starts.
    protected override void init()
    {
    }

    public override void OnEnter()
    {
        turbineToHubDoor.UnlockDoor();
        hubSwitch.activateSwitch();
    }
   
    public override void OnSuccess()
    {
        vaultDoor.UnlockDoor();
        vaultDoor.OpenDoor();
    }

    public override void UpdateQuest()
    {
        if(!hasSucceeded && hubSwitch.isSwitchActivated())
        {
            hasSucceeded = true;

            Invoke(() =>
            {
                SetQuestState(QuestState.Success);
            }, questCompletionDelay);
        }    
    }

    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }
}