// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class InteractTutorialQuest : InteractableQuest
{
    private AudioComponent_? audioComponent;
    [SerializableField]
    private MeshRenderer_? renderer;
    [SerializableField]
    private Sequence_? droppingBoxSequencer;
    [SerializableField]
    private Audio interactSFX;
    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
    }
    public override void OnEnter() {
        MapKey(Key.E, CheckInteraction);
    }

    public override void OnSuccess(){
        renderer.setMaterialFloat(0, "emissiveStrength", 9f);
        droppingBoxSequencer.play();
        audioComponent.PlaySound(interactSFX);
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