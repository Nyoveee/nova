// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ArenaQuest : Quest
{
	[SerializableField]
	private ArenaManager arenaManager;

	public override void OnEnter()
	{
		arenaManager.StartArena();
	}

	public override void OnSuccess()
	{
		Debug.Log("Arena success");
	}
	public override void OnFail()
    {
        Debug.Log("Arena failed");
		arenaManager.ResetArena();
    }

    public override void UpdateQuest()
    {

    }
}