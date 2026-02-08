// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ShootTutorialQuest : Quest
{
    [SerializableField]
    private GameObject grunt;

    [SerializableField]
    private Switch switchGameObject;

    [SerializableField]
    private float questCompleteDelay = 3f;

    private bool succeeded;

    public override void UpdateQuest() {
        if (!succeeded && grunt != null && grunt.getScript<Grunt>().IsDead())
        {
            succeeded = true;

            Invoke(() =>
            {
                SetQuestState(QuestState.Success);
                switchGameObject?.enableSwitch();

            }, questCompleteDelay);
        }
    }

}