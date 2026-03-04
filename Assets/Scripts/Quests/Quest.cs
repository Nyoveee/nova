// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public abstract class Quest : Script
{
    [SerializableField]
    private string questText;
    // State machine
    public enum QuestState
    {
        InProgress,
        Fail,
        Success
    }

    // Event for success/fail state change of quest

    public class QuestStateChangedEventArgs : EventArgs
    {
        public QuestState OldState { get; }
        public QuestState NewState { get; }

        public QuestStateChangedEventArgs(QuestState oldState, QuestState newState)
        {
            OldState = oldState;
            NewState = newState;
        }
    }
    public event EventHandler<QuestStateChangedEventArgs> OnQuestStateChanged;
    private QuestState questState = QuestState.InProgress;

    [SerializableField]
    protected Transform_? playerCheckpoint;

    /***********************************************************
        Quest Types must inherit from this
    ***********************************************************/
    public virtual void OnEnter() { }
    public virtual void OnSuccess() { }
    public virtual void OnSkip() { }
    public virtual void OnFail() { }
    public virtual void OnRestart(){}

    public virtual void UpdateQuest() { }

    public QuestState GetQuestState() => questState;

    // Should be the only way to manipulate quest state due to event invoking
    public void SetQuestState(QuestState newState)
    {
        if (questState != newState)
        {
            QuestState oldState = questState;
            questState = newState;
            OnQuestStateChanged.Invoke(this, new QuestStateChangedEventArgs(oldState, newState));
        }
    }
    public string GetQuestInformation() => questText;
    public Vector3 GetCheckpointPosition() => playerCheckpoint.position;
}