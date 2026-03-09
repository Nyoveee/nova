// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SetVisible : Script
{
    private Transform_ player;

    private MeshRenderer_ meshRenderer;
    private Rigidbody_ rigidbody;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        player = GameObject.FindWithTag("Player")?.transform;

        meshRenderer = getComponent<MeshRenderer_>();
        rigidbody = getComponent<Rigidbody_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        // OK NOT BAD ELREY NOT BAD.
        if(player.position.z > -500 && player.position.y < -200)
        {
            meshRenderer.enable = true;
            rigidbody.enable = true;
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}