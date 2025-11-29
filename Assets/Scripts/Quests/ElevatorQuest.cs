// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ElevatorQuest : Quest
{
    [SerializableField]
    private GameObject checkPointIndicator;
    [SerializableField]
    private Elevator elevator;

    public override void OnSuccess()
    {
        Destroy(checkPointIndicator);
        elevator.CloseTutorialDoor();
    }
    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }


}