// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Graphics.Display;

class RestorePowerQuest : InteractableQuest
{
    private AudioComponent_? audioComponent;
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

    [SerializableField]
    private Audio powerUnlockedSFX;

    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
    }
    // This function is first invoked when game starts.
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
        audioComponent.PlaySound(powerUnlockedSFX);
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