// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SwitchQuest : Quest
{
    [SerializableField]
    private Switch switchObject;

    [SerializableField]
    private float questCompleteDelay;

    [SerializableField]
    private Elevator elevator;

    private bool succeeded = false;

    public override void OnSuccess() {}

    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }

    public override void UpdateQuest()
    {
        if (!succeeded && switchObject != null && switchObject.isSwitchActivated())
        {
            succeeded = true;

            Invoke(() =>
            {
                SetQuestState(QuestState.Success);
                elevator?.OpenTutorialDoor();
            }, questCompleteDelay);
        }
    }

}