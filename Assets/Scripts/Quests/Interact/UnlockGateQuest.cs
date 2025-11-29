// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class UnlockGateQuest : InteractableQuest
{
    [SerializableField]
    private GameObject button;
    [SerializableField]
    private GameObject teleporter;
    // This function is first invoked when game starts.
    public override void OnEnter()
    {
        MapKey(Key.E, CheckInteraction);
    }
    public override void OnSuccess()
    {
        AudioAPI.PlaySound(gameObject, "sfx_menuClick_01");
        button.SetActive(false);
        teleporter.SetActive(false);
    }
    public override void OnFail(Transform_ playerTransform)
    {
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }
    private void CheckInteraction()
    {
        if (IsLookingAtInteractable())
            SetQuestState(QuestState.Success);
    }

}