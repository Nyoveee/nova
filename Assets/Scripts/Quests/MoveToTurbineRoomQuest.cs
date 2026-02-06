// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MoveToTurbineRoomQuest : MovementBasedQuest
{
    [SerializableField]
    private Door generatorExitDoor;

    [SerializableField]
    private Door closedRangedToHubDoor;

    [SerializableField]
    private Door hubToSewerDoor;

    public override void OnEnter()
    {
        generatorExitDoor.UnlockDoor();
        closedRangedToHubDoor.UnlockDoor();
        hubToSewerDoor.UnlockDoor();
    }

    public override void OnSuccess()
    {
    }

    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }

}