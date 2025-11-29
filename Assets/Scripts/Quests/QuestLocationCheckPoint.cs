// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public class QuestLocationCheckPoint : Script
{
    [SerializableField]
    private Quest quest;
    protected override void onCollisionEnter(GameObject other)
    {
        if (quest != null && other.tag == "Player")
            quest.SetQuestState(Quest.QuestState.Success);
    }

}