// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
public class WaypointQuest : Quest
{
	public Transform_ player;
	public Transform_ failPosition;
	public Waypoint waypointMarker;

	public override void OnEnter(){
		waypointMarker.SetWaypointQuest(this);
        Debug.Log("Quest entered.");
	}

	public override void OnSuccess()
    {
        waypointMarker.gameObject.SetActive(false);
        Debug.Log("Player walked.");
	}
	public override void OnFail()
	{
		player.position = failPosition.position;
		waypointMarker.gameObject.SetActive(true);
        Debug.Log("Player walked poorly.");
    }

    public override void UpdateQuest()
    {
		// TODO: ADD FAIL STATE UPON PLAYER DEATH
		//if (player.Die)
		// questState = 
    }
}