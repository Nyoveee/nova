// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public abstract class Quest : Script
{
    // State machine
    public enum QuestState
    {
        InProgress,
        Fail,
        Success
    }

    // Event for success/fail state change of quest
    public delegate void QuestStateChanged(QuestState oldState, QuestState newState);
    public event QuestStateChanged OnQuestStateChanged;

    private QuestState questState = QuestState.InProgress;

    /***********************************************************
        Quest Types must inherit from this
    ***********************************************************/
    public abstract void OnEnter();
    public abstract void OnSuccess();
    public abstract void OnFail();

    public abstract void UpdateQuest();

    public QuestState GetQuestState() => questState;

    // Should be the only way to manipulate quest state due to event invoking
    public void SetQuestState(QuestState newState)
    {
        if (questState != newState)
        {
            QuestState oldState = questState;
            questState = newState;
            OnQuestStateChanged.Invoke(oldState, newState);
        }
    }

}