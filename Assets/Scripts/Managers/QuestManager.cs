// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public class QuestManager : Script
{
    [SerializableField]
    private Quest[]? quests;

    private Quest? currentQuest;

    private int questIndex;

    protected override void init()
    {
        if (quests != null && quests.Length > 0)
        {
            currentQuest = quests[0];
            questIndex = 0;
        }

        if (currentQuest != null)
        {
            currentQuest.OnQuestStateChanged += HandleQuestStateChanged;
            currentQuest.OnEnter();
        }
    }

    protected override void update()
    {
        if (currentQuest != null)
            currentQuest?.UpdateQuest();
    }

    private void HandleQuestStateChanged(Quest.QuestState oldState, Quest.QuestState newState)
    {
        if (newState == Quest.QuestState.Success)
        {
            currentQuest!.OnSuccess();
            MoveToNextQuest();
        }
        else if (newState == Quest.QuestState.Fail)
        {
            currentQuest!.OnFail();
        }
    }

    private void MoveToNextQuest()
    {
        ++questIndex;
        if (questIndex == 0)
        {
            currentQuest = quests[questIndex];
        }
        else
        {
            Debug.Log("Win i guess?");
        }
    }
}