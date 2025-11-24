// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public class QuestManager : Script
{
    //public required Quest[] quests;
    // Two quests for testing purposes when without a serializable way to hold many
    [SerializableField]
    public required Quest? currentQuest;

    private int questIndex;

    protected override void init()
    {
        //if (quests != null && quests.Length > 0)
        //{
        //    //currentQuest = quests[0];
        //    questIndex = 0;
        //}

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
        // Unsure how to handle several different types of quests right now
        ++questIndex;
        if (questIndex == 0)
        {

            //currentQuest = quests[questIndex];
        }
        else
        {
            Debug.Log("Win i guess?");
        }
    }
}