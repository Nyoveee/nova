// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Ultimate : Script
{
    public Prefab ultimate;
    public Transform_ camera;

    // This function is first invoked when game starts.
    protected override void init()
    {
        MapKey(Key.E, CastUltimate);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        
    }

    private void CastUltimate()
    {
        Instantiate(ultimate);
    }
}