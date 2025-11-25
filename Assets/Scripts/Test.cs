// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Test : Script
{
    private Transform_ transform;
    public Transform_ prefab;

    public List<Prefab> prefabs;
    public List<Transform_> gameObjects;
    public List<int> transforms;
    public GameObject gameObject;

    // This function is first invoked when game starts.
    protected override void init()
    {
        Debug.Log("ok");
    }

    // This function is invoked every fixed update.
    protected override void update()
    {

    }
}