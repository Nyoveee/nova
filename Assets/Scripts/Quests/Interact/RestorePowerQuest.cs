// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class RestorePowerQuest : InteractableQuest
{
    [SerializableField]
    private MeshRenderer_ generatorCoreRenderer;
    [SerializableField]
    private List<Door> unlockableDoors;

    [SerializableField]
    private MeshRenderer_ doorIndicatorOff;
    
    [SerializableField]
    private MeshRenderer_ doorIndicatorOn;

    [SerializableField]
    public Material unlockedDoorMaterial;

    [SerializableField]
    public Material lockedDoorMaterial;

    private AudioComponent_ audioComponent;
    [SerializableField]
    private Audio interactClickSFX;

    // This function is first invoked when game starts.
    protected override void init()
    {
        base.init();
        audioComponent = getComponent<AudioComponent_>();
    }
    public override void OnEnter()
    {
        MapKey(Key.E, CheckInteraction);

        if (doorIndicatorOff != null && lockedDoorMaterial != null) 
        {
            doorIndicatorOff.changeMaterial(0, lockedDoorMaterial);
        }

        if (doorIndicatorOn != null && unlockedDoorMaterial != null)
        {
            doorIndicatorOn.changeMaterial(0, unlockedDoorMaterial);
        }
    }

    public override void OnSuccess()
    {
        // AudioAPI.PlaySound(gameObject, "sfx_menuClick_01");
        audioComponent.PlaySound(interactClickSFX);
        generatorCoreRenderer.setMaterialFloat(0, "emissiveStrength", 9f);
        foreach (Door door in unlockableDoors)
            door.UnlockDoor();
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