// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Admin : Script
{
    private PlayerController_V2 playerBody;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        playerBody = GameObject.FindWithTag("Player").getScript<PlayerController_V2>();

        // instant death.
        MapKey(Key.K, () =>
        {
            playerBody?.TakeDamage(1000);
        });
    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}