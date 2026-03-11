// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class BossHomingMissile : Enemy
{
    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField]
    public float damage = 20f;
    [SerializableField]
    public float initialiseTime = 1.0f;
    [SerializableField]
    public float maxFlightTime = 10.0f;
    [SerializableField]
    public float rotationSpeed = 10.0f;
    [SerializableField]
    public float flightSpeed = 10.0f;
    [SerializableField]
    public float flightAcceleration = 20.0f;
    [SerializableField]
    public float searchAngle = 90f;


    //[SerializableField]
    //public float m = 10.0f;

    [SerializableField]
    public float intialGravityFactor = 0.5f;
    [SerializableField]
    public float homingGravityFactor = 3.0f;


    /***********************************************************
    Runtime variables..
    ***********************************************************/
    private enum MissileState
    {
        Initialise,
        Homing,
        StraightFlight,
        Death,
    }

    /***********************************************************
    Components
    ***********************************************************/
    private MissileState missileState = MissileState.Initialise;
    private MissileStats? missileStats = null;
    private Rigidbody_ rigidbody = null;
    private AudioComponent_ audioComponent;
    private float timeElasped = 0.0f;


    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        enemyStats = getScript<EnemyStats>();
        //player = GameObject.FindWithTag("Player");
        player = GameObject.FindWithTag("PlayerHead");
  
        missileStats = getScript<MissileStats>();
        rigidbody = getComponent<Rigidbody_>();
        audioComponent = getComponent<AudioComponent_>();

        InitialiseMissileSetting(gameObject.transform.front);

        rigidbody.SetVelocityLimits(flightSpeed);
        //Debug.Log("Now facing");

    }

    // This function is invoked every update.
    protected override void update()
    {
        //set here to flush damage instantly
        if(timeElasped >= maxFlightTime)
        {
            accumulatedDamageInstance = missileStats.health;
            missileState = MissileState.Death;
        }



        FlushDamageEnemy();
        timeElasped += Time.V_DeltaTime();

        switch (missileState)
        {
            case MissileState.Initialise:
                { 
                
                }
                break;
            case MissileState.Death:
                { 
                  //Destroy(gameObject);
                    
                    gameObject.SetActive(false);

                    Invoke(() =>
                    {
                        Destroy(gameObject);
                    }, 0.1f);


                }
                break;
        }

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {

        if(player == null)
        {
            missileState = MissileState.Death;
            return;
        }

        switch (missileState)
        {
            case MissileState.Initialise:
                {
                    InitialFlight();
                }
                break;
            case MissileState.Homing:
                {
                    HomingFlight();
                }
                break;
            case MissileState.StraightFlight:
                {
                    StraightFlight();
                }
                break;
            case MissileState.Death:
                {
                  

                }
                break;
        }


    }


    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public override void TakeDamage(float damage, EnemydamageType damageType, string colliderTag)
    {
        if (damageType == Enemy.EnemydamageType.WeaponShot)
        {

            accumulatedDamageInstance += damage;


        }

        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {

                accumulatedDamageInstance += damage;
        }

        if (damageType == Enemy.EnemydamageType.Ultimate)
        {

            accumulatedDamageInstance += damage;

        }
    }

    void FlushDamageEnemy()
    {
        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();

            missileStats.health -= accumulatedDamageInstance;
            //UpdateExecutableMaterialState();
            if (missileStats.health <= 0)
            {
                if (missileState != MissileState.Death/* && !WasRecentlyDamaged()*/)
                {
                    missileState = MissileState.Death;

                }
            }
            else
            {
                //TriggerRecentlyDamageCountdown();
                //if (gruntState != GruntState.Death && !WasRecentlyDamaged())
                //{
                //    audioComponent.PlayRandomSound(hurtSFX);

                //    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
                //    renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
                //    Invoke(() =>
                //    {
                //        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
                //        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
                //    }, gruntStats.hurtDuration); //bug here is this object dies this frame
                //}
            }
            accumulatedDamageInstance = 0;
        }
    }


    public void InitialiseMissileSetting(Vector3 direction)
    {
        //missile rotates to face that direction.
        Quaternion lookRotation = Quaternion.LookRotation(direction);
        Debug.Log("Now facing " + direction);
        //Quaternion lookRotation = Quaternion.LookRotation(new Vector3( 0,1,0));
        gameObject.transform.rotation = Quaternion.Slerp(gameObject.transform.rotation, lookRotation, 1f);

        rigidbody.SetGravityFactor(intialGravityFactor);
        //Debug.Log("Now facing " + gameObject.transform.rotation);

    }


    //***** Steering functions (Fixedupdate) *********/
    public void InitialFlight()
    {
        Vector3 currentFlightPath = rigidbody.GetVelocity();
        currentFlightPath.Normalize();
        //rigidbody.SetVelocity(gameObject.transform.front * flightSpeed);
        rigidbody.AddForce(-gameObject.transform.front * flightAcceleration);
        if (timeElasped > initialiseTime)
        {
            missileState = MissileState.Homing;
            rigidbody.SetGravityFactor(homingGravityFactor);
            
        }

        

    }

    public void HomingFlight()
    { 


        Vector3 targetedFlightPath = player.transform.position - gameObject.transform.position;
        targetedFlightPath.Normalize();

        Quaternion playerTarget =  Quaternion.LookRotation(targetedFlightPath);

        Quaternion currentRotation = Quaternion.RotateTowards(gameObject.transform.rotation,playerTarget, rotationSpeed * Time.V_FixedDeltaTime());


        gameObject.transform.rotation = currentRotation;

        //rigidbody.SetVelocity(gameObject.transform.front * flightSpeed);
        rigidbody.AddForce(-gameObject.transform.front * flightAcceleration);
        rigidbody.SetGravityFactor(homingGravityFactor);

        if (isOutofRange())
        {
            missileState = MissileState.StraightFlight;

        }


    }

    public void StraightFlight()
    {
        rigidbody.AddForce(-gameObject.transform.front * flightAcceleration);
    }


    private bool isOutofRange()
    { 
        Vector3 playerDirection = player.transform.position - gameObject.transform.position;
        playerDirection.y = 0;
        playerDirection.Normalize();

        Vector3 frontDirection = gameObject.transform.front;
        //frontDirection.y = 0;
        frontDirection.Normalize();

        float dot = Vector3.Dot(frontDirection, playerDirection);
        Math.Clamp(dot, -1.0f, 1.0f);
        if ( Mathf.Rad2Deg * Math.Acos(dot) < searchAngle)
        {
            return true;
        }
        
    
        return false;
    }

    /**********************************************/

    protected override void onCollisionEnter(GameObject other)
    {

        if (other.gameObject.getComponent<Rigidbody_>().GetLayerName() == "Wall" ||
           other.gameObject.getComponent<Rigidbody_>().GetLayerName() == "Floor")
        {
             missileState = MissileState.Death;
        }

        if (other.tag == "Player")
        { 
            other.getScript<PlayerController_V2>().TakeDamage(damage);
            missileState = MissileState.Death;
            Debug.Log("Hit Player");

        }

        //Debug.Log("Collided with " + other.gameObject.name);

    }


    public override bool IsEngagedInBattle()
    {
        return true;
    }

    public override void SetSpawningDuration(float seconds)
    {

    }


}