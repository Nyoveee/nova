// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SetIBLMultiplier : Script
{
    public float multplier = 1f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        RendererAPI.iblMultiplier = multplier;
    }
}