// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
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
    private float startSteeringDistance = 10f;
    [SerializableField]
    private float maxSteeringDistance =   5f;
    [SerializableField]
    private float fakeGravity = 9.8f; //a little downward velocity so there is a throwing arc
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

    // ==================================
    // private variables
    // ==================================
    [SerializableField]
    private float calculatedTrueDamage;
    private Rigidbody_? weaponRB;

    private GameObject targetObject;
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

                    }


                }
                break;
            case ThrowingWeaponState.HitEnviroment:
                
                if(weaponSpinSequence != null)
                weaponSpinSequence.play();
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
       Vector3 directionToTarget = playerPos - gameObject.transform.position;
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


        Vector3 directionToTarget = playerPosition - gameObject.transform.position;
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
            }

        }

    }


    void DamageEnemy()
    { 
        
        targetObject.getScript<EnemyCollider>().OnColliderShot(calculatedTrueDamage,Enemy.EnemydamageType.ThrownWeapon,targetObject.tag);
    

    }

    void Receive()
    {
        playerGameobject.getScript<PlayerWeaponController>().WeaponCollected(mappedWeapon);

        //update player health
        if (playerGameobject.getScript<PlayerController>() != null)
        {

            int heal = (int)( totalIchorGained /healthGainedPerIchor);
            int sp = (int)(totalIchorGained / spGainPerIchor);

            playerGameobject.getScript<PlayerController>().GainHealth(heal);
            
            mappedWeapon.currentSp +=  sp;

            mappedWeapon.currentSp = Math.Min(mappedWeapon.currentSp,mappedWeapon.maxSp);

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
        if (mappedWeapon.currentAmmo < mappedWeapon.maxAmmo)
        {
            currentIchorGained++;
            totalAmmoGained++;

            //gain ammo and destroy object
            if (currentIchorGained >= ichorPerAmmo)
            { 
                totalAmmoGained++;
                currentIchorGained = 0;
                mappedWeapon.currentAmmo++;
            }
        
            Destroy(other);
        
        }




    }


    public void InitWeapon()
    {
        weaponRB.SetVelocity(flightPath * weaponFlyingSpeed);
        gameObject.transform.rotation = Quaternion.LookRotation(flightPath);

        calculatedTrueDamage =  throwWeaponBaseDamage  +  (mappedWeapon.maxAmmo - mappedWeapon.currentAmmo) * damageMultiplierPerAmmoUsed;

        if (weaponSpinSequence == null)
        {
            weaponSpinSequence = gameObject.GetChildren()[0].getComponent<Sequence_>();
        }
    }
}