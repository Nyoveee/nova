// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class forceTest : Script
{
    public Vector3 direction;
    public float forceStrenght;

    public float startTime = 3f;
    public float dashDuration = 1f;

    private float timer = 0f;


    public bool  isTriggerOnce = true;


    [SerializableField]
    private Rigidbody_? rb = null;

    private Transform_? transform = null;


    //private variables
    private bool isTrigger = false;
    private bool isGrounded = false;
    private bool isDashing = false;
    private bool wasInMidAir = false;
    private float dashTimeElapsed = 0f;

    protected override void init()
    { 
      rb = getComponent<Rigidbody_>();
      transform = getComponent<Transform_>();
      direction.Normalize();

    }


    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        //var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), 1f, gameObject);
        timer += Time.V_DeltaTime();
        if ( (timer > startTime) && !isTrigger)
        { 
            rb.SetVelocity(direction * forceStrenght);

            if(isTriggerOnce)
            {
                //reset trigger
                isTrigger = true;
                isDashing = true;
            }
        
        }

        //THIS CODE BREAKS the Capsule
        //if(isTrigger)
        //{
        //    if (result != null)
        //    {
        //        isGrounded = true;

        //        // this branch is only executed once, per landing..
        //        if (wasInMidAir)
        //        {
        //            wasInMidAir = false;
        //        }
        //    }

        //    if (isDashing)
        //    {
        //        handleDashing();
        //    }
        //    else
        //    {
        //        //dashTimer = Mathf.Clamp(dashTimer + Time.V_FixedDeltaTime(), 0f, dashTimerCap);
        //        handleMovement();
        //    }

        //}
        //.....
    
    }

    void handleDashing()
    {
        // We finished dashing..
        if (dashTimeElapsed > dashDuration)
        {
            isDashing = false;
           // rb.SetVelocity(Vector3.Zero());
            return;
        }

        // constantly apply velocity.
        rb.SetVelocity(direction * forceStrenght);
        //rigidbody.AddImpulse(dashVector * dashStrength);
        dashTimeElapsed += Time.V_FixedDeltaTime();
    }

    void handleMovement()
    {
        if (isGrounded)
        {
        //    rb.SetVelocity(new Vector3(0f, rb.GetVelocity().y, 0f));
        }
    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}