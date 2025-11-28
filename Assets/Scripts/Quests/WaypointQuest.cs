// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
public class WaypointQuest : Quest
{
	public Waypoint waypointMarker;

	public override void OnEnter(){
		waypointMarker.SetWaypointQuest(this);
	}

	public override void OnSuccess()
    {
        waypointMarker.gameObject.SetActive(false);
	}
	public override void OnFail(Transform_ playerTransform)
    {
        waypointMarker.gameObject.SetActive(true);
        if (playerTransform != null && playerCheckpoint != null)
        {
            playerTransform.position = playerCheckpoint.position;
        }
    }

    public override void UpdateQuest()
    {
    }
}