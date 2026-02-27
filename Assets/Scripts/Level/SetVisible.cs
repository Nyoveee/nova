// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SetVisible : Script
{
    [SerializableField]
    private GameObject ceiling;

    [SerializableField]
    private GameObject playerTrans;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {
        if(playerTrans.getComponent<Transform_>().position.z > -500 && playerTrans.getComponent<Transform_>().position.y < -200)
        {
            ceiling.getComponent<MeshRenderer_>().enable = true;
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}