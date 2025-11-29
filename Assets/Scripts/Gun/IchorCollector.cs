// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class IchorCollector : Script
{
    [SerializableField]
    ThrowableRifle weaponRef;


    //// This function is first invoked when game starts.
    protected override void init()
    { 
        weaponRef = gameObject.GetParent().getScript<ThrowableRifle>();
    }

    //// This function is invoked every update.
    //protected override void update()
    //{}

    //// This function is invoked every update.
    //protected override void fixedUpdate()
    //{}

    //// This function is invoked when destroyed.
    //protected override void exit()
    //{}

    protected override void onCollisionEnter(GameObject other)
    {
        weaponRef.ichorPull(other);
    
    
    
    }

}