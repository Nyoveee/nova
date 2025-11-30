// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class InteractableQuest : Quest
{
    [SerializableField]
    private GameObject? interactable;
    [SerializableField]
    private GameObject? player;
    [SerializableField]
    private Transform_? cameraTransform;
    // This function is first invoked when game starts.
    protected bool IsLookingAtInteractable()
    {
        RayCastResult? result = PhysicsAPI.Raycast(cameraTransform.position, cameraTransform.front, 10f, player);
        if (result == null)
            return false;
        return interactable == new GameObject(result.Value.entity);
    }
    
}