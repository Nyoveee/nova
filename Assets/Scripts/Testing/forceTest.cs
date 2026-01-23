// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class forceTest : Script
{
    public Vector3 direction;
    public float forceStrenght;

    public float startTime = 3f;

    private float timer = 0f;

    public bool  isTriggerOnce = true;

    [SerializableField]
    private Rigidbody_? rb = null;

    private bool isTrigger = false;

    protected override void init()
    { 
      rb = getComponent<Rigidbody_>();
      direction.Normalize();
    }


    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        timer += Time.V_DeltaTime();
        if ( (timer > startTime) && !isTrigger)
        { 
            rb.AddImpulse(direction * forceStrenght);

            if(isTriggerOnce)
            {
                //reset trigger
                isTrigger = true;
            }
        
        }
    
    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}