// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class BossPose_Attack : Script
{
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {




    
    }

    // This function is invoked every update.
    protected override void update()
    {
        Animator_ animator = getComponent<Animator_>();
        animator.SetFrame(38);
        animator.speedMultiplier = 0f;

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}