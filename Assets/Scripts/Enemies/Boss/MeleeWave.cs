// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MeleeWave : Script
{

    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float damage = 20.0f;
    [SerializableField]
    private float waveSpeed = 20.0f;
    [SerializableField]
    private float waveAcceleration = 60.0f;
    [SerializableField]
    private float duration = 2.0f;

    /***********************************************************
    Runtime variables..
    ***********************************************************/
    private bool hasHitPlayer = false;
    private float timeElapsed = 0.0f;
    private Rigidbody_ rigidbody = null;


    //// This function is invoked once before init when gameobject is active.
    //protected override void awake()
    //{}

    // This function is invoked once when gameobject is active.
    protected override void init()
    { 
        rigidbody = getComponent<Rigidbody_>();
        rigidbody.SetVelocityLimits(waveSpeed);
    
    }

    // This function is invoked every update.
    protected override void update()
    { 
        timeElapsed += Time.V_DeltaTime();
        if (timeElapsed > duration)
        {
            Destroy(gameObject);
        }



    }


    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        if(rigidbody != null)
        {
            rigidbody.AddForce(gameObject.transform.front * waveAcceleration);
        }

    }

    protected override void onCollisionEnter(GameObject other)
    {


        if (other.tag == "Player" && hasHitPlayer == false)
        {
            other.getScript<PlayerController_V2>().TakeDamage(damage);
            hasHitPlayer = true;
            Debug.Log("Hit Player");
        }

    }




    //// This function is invoked when destroyed.
    //protected override void exit()
    //{}

}