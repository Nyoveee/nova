// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Test : Script
{
    private Transform_ transform;
    public Prefab prefab;
    // This function is first invoked when game starts.
    protected override void init()
    {

    }

    // This function is invoked every fixed update.
    protected override void update()
    {

    }

    public void foo()
    {
        //Debug.Log("hey");
        
        //GameObject instance = Instantiate(prefab, new Vector3(-5, 0, 0), gameObject);
        //Debug.Log(instance.transform.position);
    }
}