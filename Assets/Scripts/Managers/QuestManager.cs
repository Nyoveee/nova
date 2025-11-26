// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public class QuestManager : Script
{
    private List<Quest> quests = new List<Quest>();

    private Quest? currentQuest;

    private int questIndex;

    protected override void init()
    {
        GameObject[] children = gameObject.GetChildren();
        foreach (var child in children)
        {
            Quest quest = child.getScript<Quest>();
            if (quest != null) {
                quests.Add(quest);
            }
            else {
                Debug.LogWarning("Quest child of object " + gameObject.ToString() + " does not have quest script");
            }
        }

        if (quests != null && quests.Count > 0)
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
        if (newState == oldState)
        {
            Debug.Log("Quest new/old states same");
            return;
        }

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
        if (questIndex < quests.Count)
        {
            currentQuest = quests[questIndex];
            currentQuest.OnEnter();
        }
        else
        {
            Debug.Log("Player Won/Quests are done");
        }
    }
}