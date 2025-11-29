// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class InteractTutorialQuest : Quest
{
    [SerializableField]
    private GameObject? interactable;
    [SerializableField]
    private GameObject? player;
    [SerializableField]
    private MeshRenderer_? renderer;
    [SerializableField]
    private Transform_? cameraTransform;
    [SerializableField]
    private Sequence_? droppingBoxSequencer;
    // This function is first invoked when game starts.
    public override void OnEnter() {
        MapKey(Key.E, CheckInteraction);
    }

    public override void OnSuccess(){
        renderer.setMaterialFloat(6, "intensity", 9f);
        droppingBoxSequencer.play();
        AudioAPI.PlaySound(gameObject, "sfx_menuClick_01");
    }
    public override void OnFail(Transform_ playerTransform){
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }

    public override void UpdateQuest() { }
    private bool IsLookingAtGenerator()
    {
        RayCastResult? result = PhysicsAPI.Raycast(cameraTransform.position, cameraTransform.front, 1000f, player);
        if (result == null)
            return false;
        return interactable == new GameObject(result.Value.entity);
    }
    private void CheckInteraction()
    {
        if (IsLookingAtGenerator())
            SetQuestState(QuestState.Success);
    }

}