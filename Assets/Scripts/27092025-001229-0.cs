// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlaySound : Script
{
    public Transform_ transform;

    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {}

    protected override void onCollisionEnter(uint entityId) {
        AudioAPI.PlaySound(entityID, "metal_pipe");
        AudioAPI.PlaySound(entityID, "SCREAM");
    } 
}