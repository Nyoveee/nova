// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class InteractTutorialQuest : InteractableQuest
{
    [SerializableField]
    private MeshRenderer_? renderer;
    [SerializableField]
    private Sequence_? droppingBoxSequencer;
    // This function is first invoked when game starts.
    public override void OnEnter() {
        MapKey(Key.E, CheckInteraction);
    }

    public override void OnSuccess(){
        renderer.setMaterialFloat(0, "emissiveStrength", 9f);
        droppingBoxSequencer.play();
        // AudioAPI.PlaySound(gameObject, "sfx_menuClick_01");
    }
    public override void OnFail(Transform_ playerTransform){
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }
    private void CheckInteraction()
    {
        if (IsLookingAtInteractable())
            SetQuestState(QuestState.Success);
    }


}