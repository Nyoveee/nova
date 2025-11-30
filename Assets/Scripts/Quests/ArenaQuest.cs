// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
public class ArenaQuest : Quest
{
	[SerializableField]
    private ArenaManager arenaManager;
	[SerializableField]
    private List<Door> unlockableDoors;
    public override void OnEnter()
	{
		arenaManager.StartArena(this);
	}

	public override void OnSuccess()
	{
		Debug.Log("Arena success");
        foreach (Door door in unlockableDoors)
            door.UnlockDoor();
    }
	public override void OnFail(Transform_ playerTransform)
    {
        Debug.Log("Arena failed");
		arenaManager.ResetArena();
		if (playerTransform != null && playerCheckpoint != null)
		{
			playerTransform.position = playerCheckpoint.position;
		}
    }

    public override void UpdateQuest()
    {
    }
}