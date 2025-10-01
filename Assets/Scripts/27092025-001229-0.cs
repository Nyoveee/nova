// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlaySound : Script
{
    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {}

    protected override void onCollisionEnter(GameObject other) {
        AudioAPI.PlaySound(gameObject, "METAL PIPE");
        AudioAPI.PlaySound(gameObject, "SCREAM");
    } 
}