// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
public class ArenaQuest : Quest
{
	[SerializableField]
    private ArenaManager arenaManager;
    
    public override void OnEnter()
	{
		arenaManager.StartArena(this);
	}

	public override void OnSuccess()
	{
    }

    public override void UpdateQuest()
    {
    }
}