// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public class Waypoint : Script
{
    private WaypointQuest waypointQuest;

    public void SetWaypointQuest(WaypointQuest quest) {  waypointQuest = quest; }

    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Player")
        {
            waypointQuest.SetQuestState(Quest.QuestState.Fail);
        }
    }

}