// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.ComponentModel;
using static ThrowableRifle;

class ThrowableRifle : Script
{

    //Manages the throwing weapon flight controls and damage.

    // ==================================
    // References
    // ==================================
    [SerializableField]
    public Vector3 flightPath;
    [SerializableField] //current weapon in used
    public Gun mappedWeapon; //What is the weapon in player inventory
    public GameObject playerGameobject;
    [SerializableField] //current weapon in used
    public Sequence_ weaponSpinSequence;

    // ==================================
    // Parameters
    // ==================================
    [SerializableField]
    private Prefab contactSparkVFXPrefab;
    [SerializableField]
    private ColorAlpha weakPointHitSparkColour;
    [SerializableField]
    private float maxFlyingTime = 1f;
    [SerializableField]
    private float maxReturnTime = 1f;
    [SerializableField]
    private float weaponHitDelayTime = 0.4f;
    [SerializableField]
    private float weaponFlyingSpeed = 60f;
    [SerializableField]
    private float weaponReturnSpeed = 45f;
    [SerializableField]
    private float throwWeaponBaseDamage = 50f;
    [SerializableField]
    private float damageMultiplierPerAmmoUsed = 1.3f;
    [SerializableField]
    private float returnDamage = 20f;
    [SerializableField]
    private float baseSeekStrength = 30f; //rotatation strength in degrees
    [SerializableField]
    private float startSteeringDistance = 50f;
    [SerializableField]
    private float maxSteeringDistance =   40f;
    [SerializableField]
    private float seekDistance = 3f;
    [SerializableField]
    private float fakeGravity = 9.8f; //a little downward velocity so there is a throwing arc
    [SerializableField]
    private float angledFlight = 1f; //a little downward velocity so there is a throwing arc
    [SerializableField]
    private int ichorPerAmmo = 2; //gain X amount of ammo per ichor
    [SerializableField]
    private int healthGainedPerIchor = 2; //gain X amount of health per ichor
    [SerializableField]
    private int spGainPerIchor = 2; //gain X amount of sp per ichor
    [SerializableField]
    private int ichorFindRadius = 5; //gain X amount of sp per ichor

    private float currentIchorGained =0;
    private float totalIchorGained = 0;
    private float currentAmmoGained = 0;
    private float totalAmmoGained = 0;

    [SerializableField]
    private Audio pickupSFX;
    [SerializableField]
    private List<Audio> throwSFX;
    [SerializableField]
    private List<Audio> hitWallSFX;
    [SerializableField]
    private List<Audio> hitSFX;
    // ===========================================
    // Components
    // ===========================================
    private AudioComponent_ audioComponent;

    // ==================================
    // Private Variables
    // ==================================
    [SerializableField]
    private float calculatedTrueDamage;
    private Rigidbody_? weaponRB;

    private GameObject targetObject = null;
    private ThrowingWeaponState throwingWeaponState;
    private bool seekingFailed = true; //Need to call seeking
    private float timeElapsed = 0f;
    private List<GameObject> hasDamageList = new List<GameObject>();
    private float playerHeight =5.0f;


    public enum ThrowingWeaponState
    { 
        Seeking,
        Flying,
        HitEnviroment,
        HitEnemy,
        HitDelay,
        Return,
        Recieve
    }



    // This function is first invoked when game starts.
    protected override void init()
    {
        weaponRB = getComponent<Rigidbody_>();
        audioComponent = getComponent<AudioComponent_>();
        //var gameObjectsChild  = gameObject.GetChildren();

        //foreach (var gameObject in gameObjectsChild)
        //{
        //    if (gameObject.getComponent<Sequence_>() != null)
        //    {
        //        weaponSpinSequence = gameObject.getComponent<Sequence_>();
        //    }
        //}

        throwingWeaponState = ThrowingWeaponState.Seeking;
        



    }

    public void InitWeapon()
    {

        flightPath.Normalize();
        angledFlight *= Mathf.Deg2Rad;
        Vector3 angledFlightPath = Quaternion.AngleAxis(angledFlight, Vector3.Cross(Vector3.Up(), flightPath)) * flightPath;

        

        weaponRB.SetVelocity(angledFlightPath * weaponFlyingSpeed);
        angledFlightPath.Normalize();
        gameObject.transform.rotation = Quaternion.LookRotation(-angledFlightPath);

        

        //Quaternion baseRotation = Quaternion.LookRotation(flightPath);
        //Quaternion angleOffset = Quaternion.Euler(angledFlight, 0f, 0f);

        //Quaternion finalRotation = baseRotation * angleOffset;

        //// The new direction is the forward vector of this final rotation.
        //Vector3 angledFlightPath = finalRotation * Vector3.forward;

        // The rest of your launch code remains the same:
        //weaponRB.SetVelocity(angledFlightPath * weaponFlyingSpeed);
        //gameObject.transform.rotation = finalRotation

        calculatedTrueDamage = throwWeaponBaseDamage + (mappedWeapon.MaxAmmo - mappedWeapon.CurrentAmmo) * damageMultiplierPerAmmoUsed;

        if (weaponSpinSequence == null)
        {
            weaponSpinSequence = gameObject.GetChildren()[0].getComponent<Sequence_>();
        }




    }

    // This function is invoked every update.
    protected override void update()
    {
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        timeElapsed += Time.V_FixedDeltaTime();

        switch (throwingWeaponState)
        { 
            case ThrowingWeaponState.Seeking:
                {

                    throwingWeaponState = ThrowingWeaponState.Flying;

                }
                break;
            case ThrowingWeaponState.Flying:
                { 
                    if(timeElapsed > maxFlyingTime)
                    {
                        weaponSpinSequence.play();
                        throwingWeaponState = ThrowingWeaponState.HitDelay;
                        timeElapsed = 0f;
                        LookAtPlayer();
                    }

                    //Continue in trajctory if it fails to seek, set artifical gravity 
                    if (seekingFailed == true)
                    {

                        //Vector3 rbVelocity = weaponRB.GetVelocity();
                        //rbVelocity.y -= 1.0f;

                        //rbVelocity = rbVelocity * weaponFlyingSpeed * Time.V_FixedDeltaTime();
                        //weaponRB.SetVelocity(rbVelocity);


                        Vector3 v = weaponRB.GetVelocity();
                        v.y -= fakeGravity * Time.V_FixedDeltaTime();
                        weaponRB.SetVelocity(v);
                    }
                    else
                    {
                        HomingToTarget();
                    
                    }


                }
                break;
            case ThrowingWeaponState.HitEnviroment:
                
                if(weaponSpinSequence != null)
                weaponSpinSequence.play();
                //audioComponent.PlayRandomSound(hitWallSFX);
                throwingWeaponState = ThrowingWeaponState.HitDelay;
                timeElapsed = 0;
                break;
            case ThrowingWeaponState.HitEnemy:

                if (weaponSpinSequence != null)
                    weaponSpinSequence.play();
                DamageEnemy();
                throwingWeaponState = ThrowingWeaponState.HitDelay;
                timeElapsed = 0;
                break;
            case ThrowingWeaponState.HitDelay:
                {
                    HitDelay();
                
                }
                break;
            case ThrowingWeaponState.Return:
                {
                    SteerTowardsPlayerByTime();



                }
                break;
            case ThrowingWeaponState.Recieve:
                Receive();
                break;
        
        
        
        
        
        }
    }

    private void SteerTowardsPlayerByTime()
    {
        //Vector3 currentVelocity = weaponRB.GetVelocity() + gameObject.transform.front * weaponReturnSpeed;

        //if (currentVelocity.Length() > weaponReturnSpeed)
        //{
        //   currentVelocity = gameObject.transform.front * weaponReturnSpeed;

        //}

       Vector3 playerPos =  playerGameobject.transform.position;
       playerPos.y += playerHeight;
       Vector3 directionToTarget = gameObject.transform.position - playerPos;
        directionToTarget.Normalize();

        float steerPower = timeElapsed / maxReturnTime;

        if (steerPower > 1.0f)
        {
            steerPower = 1.0f;
        }   

        float rotationSpeed = baseSeekStrength + (360 - baseSeekStrength) * steerPower;

        float rotationFactor = rotationSpeed / 360.0f;

        if (rotationFactor > 1.0f)
        {
            rotationFactor = 1.0f;
        }


        Quaternion targetRotation = Quaternion.LookRotation(directionToTarget);
        Quaternion newRotation = Quaternion.Slerp(gameObject.transform.rotation, targetRotation, rotationFactor);


        gameObject.transform.rotation = newRotation;
        //weaponRB.SetBodyRotation(targetRotation);


        Vector3 localFront = gameObject.transform.rotation * Vector3.Front();

        Vector3 currentVelocity = localFront * weaponReturnSpeed;

        weaponRB.SetVelocity(currentVelocity);
    }



    void HomingToTarget()
    {
        if (targetObject != null)
        {
            seekingFailed = true;

        }

        Vector3 directionToTarget = (targetObject.transform.position - gameObject.transform.position);

        directionToTarget.Normalize();

        float distance = Vector3.Distance(targetObject.transform.position, gameObject.transform.position);

        float steerPower = (startSteeringDistance - distance) / (startSteeringDistance - maxSteeringDistance);


        if (steerPower > 1.0f)
        {
            steerPower = 1.0f;
        }

        if(steerPower < 0.0f)
        {
            steerPower = 0.0f;
        }

        //always up to max rotation
        float rotationSpeed = baseSeekStrength + (360 - baseSeekStrength) * steerPower;

        float rotationFactor = rotationSpeed / 360.0f;

        if (rotationFactor > 1.0f)
        {
            rotationFactor = 1.0f;
        }

        Quaternion targetRotation = Quaternion.LookRotation(directionToTarget);

        Quaternion newRotation = Quaternion.Slerp(gameObject.transform.rotation, targetRotation, rotationFactor);


        gameObject.transform.rotation = newRotation;


        Vector3 localFront = gameObject.transform.rotation * Vector3.Front();

        Vector3 currentVelocity = localFront * weaponFlyingSpeed;

        weaponRB.SetVelocity(currentVelocity);

    }


    private void HitDelay()
    {
        if (timeElapsed > weaponHitDelayTime)
        {

            throwingWeaponState = ThrowingWeaponState.Return;
            LookAtPlayer();
            timeElapsed = 0;
            hasDamageList.Clear();
        }


    }

    private void LookAtPlayer()
    {  
        Vector3 playerPosition = playerGameobject.transform.position;
        playerPosition.y += playerHeight;


        Vector3 directionToTarget = gameObject.transform.position - playerPosition;
        directionToTarget.Normalize();
       
       gameObject.transform.rotation = Quaternion.LookRotation(directionToTarget);
    }


    protected override void onCollisionEnter(GameObject other)
    {

        if (other.tag == "Player" && throwingWeaponState == ThrowingWeaponState.Return)
        {
            weaponRB.SetVelocity(Vector3.Zero());
            throwingWeaponState = ThrowingWeaponState.Recieve;
        
        }


        if ( (other.tag == "Wall" || other.tag == "Floor") && throwingWeaponState == ThrowingWeaponState.Flying)
        {
            //audioComponent.PlayRandomSound(hitWallSFX);
            weaponRB.SetVelocity(Vector3.Zero());
            throwingWeaponState = ThrowingWeaponState.HitEnviroment;

        }

        if ((other.tag == "Enemy_ArmouredSpot" || other.tag == "Enemy_WeakSpot") && throwingWeaponState == ThrowingWeaponState.Flying)
        {
            weaponRB.SetVelocity(Vector3.Zero());
            targetObject = other;
            throwingWeaponState = ThrowingWeaponState.HitEnemy;

        }


        if ((other.tag == "Enemy_ArmouredSpot" || other.tag == "Enemy_WeakSpot") && throwingWeaponState == ThrowingWeaponState.Return)
        {


            GameObject candidateobject  = other.GetParent();
            if (hasDamageList.Contains(candidateobject) == false)
            {
                hasDamageList.Add(candidateobject);
                other.getScript<EnemyCollider>().OnColliderShot(returnDamage, Enemy.EnemydamageType.ThrownWeapon, other.tag);
                GameObject contactSparkVFX = Instantiate(contactSparkVFXPrefab, gameObject.transform.position);
                contactSparkVFX.getComponent<ParticleEmitter_>().emit();
            }

        }

    }


    void DamageEnemy()
    {
        //audioComponent.PlayRandomSound(hitSFX);
        targetObject.getScript<EnemyCollider>().OnColliderShot(calculatedTrueDamage,Enemy.EnemydamageType.ThrownWeapon,targetObject.tag);
        Vector3 direction = targetObject.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        // LookRotation is based on Z axis, rotate the emitter to face the z axis first
        GameObject contactSparkVFX = Instantiate(contactSparkVFXPrefab, gameObject.transform.position,Quaternion.LookRotation(direction) * Quaternion.AngleAxis(Mathf.Deg2Rad * 90, new Vector3(1, 0, 0)));
        contactSparkVFX.getComponent<ParticleEmitter_>().emit();

    }

    void Receive()
    {
        playerGameobject.getScript<PlayerWeaponController>().WeaponCollected(mappedWeapon);
        //audioComponent.PlaySound(pickupSFX);

        //update player health
        if (playerGameobject.getScript<PlayerController>() != null)
        {

            int heal = (int)( totalIchorGained /healthGainedPerIchor);
            int sp = (int)(totalIchorGained / spGainPerIchor);

            playerGameobject.getScript<PlayerController>().GainHealth(heal);
            
            mappedWeapon.CurrentSp +=  sp;

            mappedWeapon.CurrentSp = Math.Min(mappedWeapon.CurrentSp,mappedWeapon.MaxSp);

        }

        Destroy(gameObject);

    }

    public void ichorPull(GameObject other)
    {
        if (other.tag != "Ichor")
        {
            return;
        }


        //Continue to gain ichor until max ammo
        if (mappedWeapon.CurrentAmmo < mappedWeapon.MaxAmmo)
        {
            currentIchorGained++;
            totalAmmoGained++;

            //gain ammo and destroy object
            if (currentIchorGained >= ichorPerAmmo)
            { 
                totalAmmoGained++;
                currentIchorGained = 0;
                mappedWeapon.CurrentAmmo++;

            }


            if (other.getScript<Ichor>() != null)
            {
                other.getScript<Ichor>().PullTowardsGun(gameObject.transform.position);



            }

          //  Destroy(other);
        
        }




    }
    
    
    
    
    public void SeekTarget(Vector3 origin, Vector3 end)
    {
        GameObject[] candidateEnemies = GameObject.FindGameObjectsWithTag("Enemy_WeakSpot");

        Vector3 rayCast = end - origin;

        GameObject candidateTarget = null;

        float smallestDistance = float.PositiveInfinity;

        foreach (var enemy in candidateEnemies)
        {
            Vector3 otherVector = enemy.transform.position - origin;

            Vector3 pointOnLine = Vector3.Proj(otherVector, rayCast);

            if (Vector3.Distance(pointOnLine + origin, enemy.transform.position) > seekDistance)
            {
                continue;
            }

            float t = Vector3.Dot(pointOnLine, rayCast);


            //is on line segment?
            if (t > 0 && pointOnLine.Length() < rayCast.Length())
            {
                float currentDistance = Vector3.Distance(pointOnLine + origin, enemy.transform.position);

                if (currentDistance < smallestDistance)
                {

                    smallestDistance = currentDistance;
                    candidateTarget = enemy;
                }

            }




        }


        if (candidateTarget != null)
        {
            seekingFailed = false;
            targetObject = candidateTarget;
        }

    }




}