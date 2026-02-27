// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Hiding : Script
{
    [SerializableField]
    private GameObject lever;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {
        if (lever.getScript<Switch>().isSwitchActivated() == true)
        {
            Vector3 pos = gameObject.transform.position;

            if (pos.z > -1500)
            {
                //50
                pos.z -= 200 * Time.V_DeltaTime();
                gameObject.transform.position = pos;
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}