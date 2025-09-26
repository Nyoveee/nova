// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Physics : Script
{
    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {}

    protected override void onCollisionEnter(uint entityId) {
        Debug.Print("This is what I dreamed of reality.\n");
        Debug.Print("Collide with entity " + entityId + "\n");
    } 
}