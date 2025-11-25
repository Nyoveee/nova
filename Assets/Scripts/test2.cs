// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class test2 : Script
{
    private Transform_ transform;
    // This function is first invoked when game starts.
    protected override void init()
    {
        Debug.Log("sup squad");
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
    }

    public void Somewhere()
    {
        Debug.Log("animation event 1");
    }

    public void SomewhereAgain()
    {
        Debug.Log("animation event 2");
    }
}