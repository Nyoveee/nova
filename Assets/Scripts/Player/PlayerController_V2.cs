// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class PlayerController_V2 : Script
{
    // ==================================
    // Parameters
    // ==================================
    [SerializableField]
    private float groundAcceleration = 2f; //rate of change of player speed
    [SerializableField]
    private float groundMaxMoveSpeed = 10f; //the max speed/ the desirable speed of the player on ground
    [SerializableField]
    private float groundDrag = 20.0f; //deceleration
    [SerializableField]
    private float groundDetectionRayCast = 3.0f; //raycast detection distance to ground. ideally to crosscheck with player height. (Not this is not use for airborne/ground movement checking)
    [SerializableField]
    private float airAcceleration = 2f; //rate of change of player speed
    [SerializableField]
    private float aerialMaxMoveSpeed = 8f; //the max speed/ the desirable speed of the player in air
    //[SerializableField]
    //public float aerialResistiveForceFactor = 1f; //resistive force when the player is moving too fast
    [SerializableField]
    private float airDrag = 0f; //lost of speed per second when in the air, note this should be zero at all times??? otherwise this messes up gravity as well, so player falls slower than intended
    [SerializableField]
    private float jumpStrength =   1000f;
    [SerializableField]
    private float jumpGravityFactor = 1f; //should be =< than base gravity factor
    [SerializableField]
    private float lerpGravityFactorDuration = 1f;
    //[SerializableField]
    //public float dashStrength = 10f;
    [SerializableField]
    private float desiredDashSpeed = 100f;
    [SerializableField]
    public float dashDuration = 1f;
    [SerializableField]
    public float dashCooldown = 0.5f;
    [SerializableField]
    private float speedChangeSmoothTime = 0.1f;
    [SerializableField]
    private float baseGravityFactor = 1f; //gravity factor for groundbased movements



    // ==================================
    // Events
    // ==================================
    public event EventHandler OnPlayerDeath;


    // ==================================
    // References
    // ==================================
    private GameUIManager gameUIManager;
    
    [SerializableField]
    private Transform_? playerOrientation = null;
    
    [SerializableField]
    private PlayerMotionAnimations? playerMotionAnimations = null;
    private Transform_? transform;
    private Rigidbody_? rigidbody;
    private AudioComponent_? audioComponent;

    // ==================================
    // Runtime Variables
    // ==================================
    // WASD
    public PlayerMoveStates playerMoveStates = PlayerMoveStates.Disabled;
    private bool isMovingForward = false;
    private bool isMovingBackward = false;
    private bool isMovingLeft = false;
    private bool isMovingRight = false;

    //bools
    private bool isGrounded = false;
    private bool wasGrounded = false;
    private bool jumpEnabled = false;
    private bool onDashTrigger = false;
    private bool isLerpingGravity = false;
    private bool isDashKeyHeld = false; //we dw to chain dash if key is held down.
    private float jumpCD = 0.5f;
    private int contactSurfaces = 0;
    private bool isLanded = true;

    private Vector3 directionVector = Vector3.Zero();
    private float   desiredSpeed = 0.0f; //the speed of the player the player should move at.
    private bool    isIFrames = false;
    private Vector3 restoreVelocity = Vector3.Zero(); //used for ulti only please :( unless you need other things dont touch this

    //Timers
    private float jumpTimer = 0.0f;
    private float dashCooldownTimer = 0.0f;
    private float gravityLerpTimer = 0.0f;
    private float dashTimeElapsed = 0.0f;

    public bool movementIsEnabled = true;

    // ==================================
    // Player Stats
    // ==================================
    [SerializableField] //health
    public float maxHealth = 100f;
    private float currentHealth = 0f;
    [SerializableField]
    public float maxStamina = 3f;
    [SerializableField]
    public float dashStaminaConsumption = 1f;
    [SerializableField]
    public float dashStaminaRecovery = 1.0f;
    public float currentStamina;

    //public bool ToEnable = true;


    // Hurt & Jump
    private int hitsBeforeSound = 3;
    private int hitsTaken = 0;
    private int jumpsBeforeSound = 5;
    private int jumpsDone = 0;

    // Audio
    [SerializableField]
    private List<Audio> deathSFX;
    [SerializableField]
    private List<Audio> hurtSFX;
    [SerializableField]
    private List<Audio> dashSFX;
    [SerializableField]
    private List<Audio> jumpVOSFX;
    [SerializableField]
    private List<Audio> jumpSFX;
    [SerializableField]
    private List<Audio> footstepSFX;
    [SerializableField]
    private List<Audio> landingSFX;
    [SerializableField]
    private float timeBetweenSteps = 0.36f;
    private float timeSinceLastFootstep = 0f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        playerMoveStates = PlayerMoveStates.InitState;
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();
        audioComponent = getComponent<AudioComponent_>();
        playerMotionAnimations = getScript<PlayerMotionAnimations>();

        //TBH should get other system to check for lockedmouse
        CameraAPI.LockMouse();

        // Health
        currentHealth = maxHealth;
        currentStamina = maxStamina;

        rigidbody.SetGravityFactor(baseGravityFactor);
        

        //Input initialisation
        MapKey(Key.W, beginWalkingForward, endWalkingForward);
        MapKey(Key.A, beginWalkingLeft, endWalkingLeft);
        MapKey(Key.S, beginWalkingBackward, endWalkingBackward);
        MapKey(Key.D, beginWalkingRight, endWalkingRight);

        MapKey(Key.Space, triggerJump);

        MapKey(Key.LeftShift, triggerDash, dashkeyUpHandler);

        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        isIFrames = false;
    }

    // This function is invoked every update.
    protected override void update()
    {
        currentStamina = Mathf.Clamp(currentStamina + Time.V_DeltaTime() * dashStaminaRecovery, 0f, maxStamina);

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        //Add to timer
        jumpTimer += Time.V_FixedDeltaTime();
        dashCooldownTimer += Time.V_FixedDeltaTime();

        // Debug.Log("Horizontal Velocity: " + GetCurrentHorizontalVelocity());

        //Debug.Log(contactSurfaces);
        //Debug.Log("Jump Speed: " + rigidbody.GetVelocity().y);

        //gravitylerp for jumps
        if (isLerpingGravity)
        {
            gravityLerpTimer += Time.V_FixedDeltaTime();

            if (gravityLerpTimer >= lerpGravityFactorDuration)
            {
                rigidbody.SetGravityFactor(baseGravityFactor);
                isLerpingGravity = false;
                gravityLerpTimer = 0f;
            } 
            else
            {
            rigidbody.SetGravityFactor(float.Lerp(jumpGravityFactor, baseGravityFactor, gravityLerpTimer / lerpGravityFactorDuration));
            }
        }

        //if speed is too fast we want to slow player down
        SpeedModulator();


        if (PlayerMoveStates.InitState != playerMoveStates &&
            PlayerMoveStates.StartJump != playerMoveStates &&
            PlayerMoveStates.Jumping   != playerMoveStates &&
            PlayerMoveStates.Dashing   != playerMoveStates)
        {
            CheckMovementTypeState(); //do something when the player is on the ground and not on the ground,
            //we should check gravity controls here as well
        }


        //Main update loop player movement states
        switch (playerMoveStates)
        {
            case PlayerMoveStates.Disabled:
                {
                    //Probably Nothing here
                }
                break;
            case PlayerMoveStates.InitState:
                {
                    //Player should be on the ground before being able to move
                    InitMovement();
                }
                break;

            case PlayerMoveStates.GroundedMovement:
                {
                    UpdateGroundedMovement();
                }
                break;
            case PlayerMoveStates.AirborneMovement:
                {
                    UpdateAirborneMovement();

                }
                break;
            case PlayerMoveStates.Dashing:
                {
                    if (onDashTrigger == true)
                    {
                        InitiateDashing();
                    }
                    Dashing();
                }
                break;
            case PlayerMoveStates.StartJump:
                {
                    InitiateJump();
                }
                break;
            case PlayerMoveStates.Jumping: //we need one frame of jump, to prevent grounded from being called
                {
                    playerMoveStates = PlayerMoveStates.AirborneMovement;
                }
                break;
            case PlayerMoveStates.Death:
                {

                }
                break;
        }

        if (gameUIManager != null)
            gameUIManager.SetProgress(GameUIManager.ProgressBarType.DashBar, currentStamina, maxStamina);

    }
    void CheckMovementTypeState()
    {
        //NOTE due to using raycast to check for groundedness, there is a probably a couple of frame where
        // we will enable airborne movement but account for jump recovery. 
        //This does not gurantee to solve the lost jump bug. in a scenario that the raycast is too close to the ground and jump
        // is triggred, the player will be able to jump again. player does not leave the ground. (give back jump after a short cd?)

        //string[] mask = { "Enemy_HurtSpot", "NonMoving", "Floor" };
        //var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), 1f, mask);

        //if (result != null)
        //{
        //    isGrounded = true;
        //   // Debug.Log("Grounded");
        //}
        //else
        //{
        //    isGrounded = false;
        //    Debug.Log("Airborne");
        //}

        //if (isGrounded == true && wasGrounded == false)
        //{
        //    //player as landed on the ground
        //    playerMoveStates = PlayerMoveStates.GroundedMovement;
        //    rigidbody.SetLinearDamping(groundDrag);
        //    rigidbody.SetVelocityLimits(groundMaxMoveSpeed);
        //    jumpCount = 1;
        //    wasGrounded = true;
        //    Debug.Log("Grounded");

        //}

        //if (isGrounded == false && wasGrounded == true)
        //{
        //    //player has just left the ground
        //    playerMoveStates = PlayerMoveStates.AirborneMovement;
        //    rigidbody.SetLinearDamping(0);
        //    rigidbody.SetVelocityLimits(aerialMaxMoveSpeed);
        //    wasGrounded = false;
        //}

        //is in contact with one or more valid surfaces
        if (contactSurfaces > 0)
        {
            triggerLandSFX();
            playerMoveStates = PlayerMoveStates.GroundedMovement;
            rigidbody.SetLinearDamping(groundDrag);
            rigidbody.SetVelocityLimits(groundMaxMoveSpeed);
            jumpEnabled = true;
            //Debug.Log("Grounded");

        }
        else
        {
            playerMoveStates = PlayerMoveStates.AirborneMovement;
            rigidbody.SetLinearDamping(airDrag);
            rigidbody.SetVelocityLimits(100000); //player movespeed is uncapped in air
            jumpEnabled = false;
            //Debug.Log("Airborne");
        }
    }

    //First time initialisation of play movement
    void InitMovement()
    {
        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Floor" };
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), groundDetectionRayCast, mask);

        if (result == null)
        {
            return;
        }

        isGrounded = true;
        wasGrounded = true;
        playerMoveStates = PlayerMoveStates.GroundedMovement;
        jumpEnabled = true;


        rigidbody.SetLinearDamping(groundDrag);
        rigidbody.SetVelocityLimits(groundMaxMoveSpeed);

    }
    void UpdateMovementVector()
    {
        // ==============================
        // Handles WASD movement, we calculate the oriented directional vector from input..
        // ==============================
        Vector3 orientedFront = new Vector3(playerOrientation.front.x, 0, playerOrientation.front.z);
        Vector3 orientedRight = new Vector3(playerOrientation.right.x, 0, playerOrientation.right.z);

        directionVector = Vector3.Zero();

        orientedFront.Normalize();
        orientedRight.Normalize();

        // We accumulate all directional input command..
        bool isMoving = false;

        if (isMovingForward)
        {
            directionVector += orientedFront;
            isMoving = true;
        }
        if (isMovingBackward)
        {
            directionVector -= orientedFront;
            isMoving = true;

        }
        if (isMovingLeft)
        {
            directionVector -= orientedRight;
            isMoving = true;

        }
        if (isMovingRight)
        {
            directionVector += orientedRight;
            isMoving = true;
        }

        if (isMoving == false)
        {
            directionVector = Vector3.Zero();
        }
    }

    void UpdateGroundedMovement()
    {

        UpdateMovementVector();
        desiredSpeed = groundMaxMoveSpeed;
        HandleFootstepSound();
        //alter direction vector if player is on a slope
        string[] mask = {"NonMoving", "Floor" };
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), groundDetectionRayCast, mask);
        if (result != null)
        {
            //projection onto a plane 
            //Forumla: deisred vector = vector - (vector dot plane normal) * plane normal

            Vector3 planeNormal = result.Value.hitSurfaceNormal;
            Vector3 desiredVector = directionVector - Vector3.Dot(directionVector, planeNormal) * planeNormal;
            directionVector = desiredVector;

        }

        //if stil in lerping gravityfactor change gravity to jump gravity now
        if (isLerpingGravity)
        {
            //Debug.Log("STOPPED LERPING GRAVITY FACTOR");
            rigidbody.SetGravityFactor(baseGravityFactor);
            isLerpingGravity = false;
            gravityLerpTimer = 0f;
        }


        //we work with forward MS first 
        if (directionVector != Vector3.Zero())
        {
            rigidbody.AddImpulse(directionVector * groundAcceleration);
            rigidbody.SetLinearDamping(0);
        }
        else
        {
            rigidbody.SetLinearDamping(groundDrag);
        }


    }

    void UpdateAirborneMovement()
    {
        UpdateMovementVector();


        desiredSpeed = aerialMaxMoveSpeed;

        //we work with forward MS first 
        if (directionVector != Vector3.Zero())
        {
            //rigidbody.AddForce(directionVector * playerAccleration);
            Vector3 currentHorizontalVelocity = rigidbody.GetVelocity();
            currentHorizontalVelocity.y = 0f;


            bool isAllowedtoAddForce = true;

            //do a quick check if applying
            Vector3 forceapplied = directionVector * airAcceleration;

            Vector3 predictedVelocity = currentHorizontalVelocity + (forceapplied/rigidbody.GetMass()) * Time.V_FixedDeltaTime();

            //player is moving faster than allowed, stop accelerating
            if (predictedVelocity.Length() < aerialMaxMoveSpeed)
            {
                rigidbody.AddForce(directionVector * airAcceleration);
            }
            //    rigidbody.SetLinearDamping(0);
            //}
            //else
            //{
            //    rigidbody.SetLinearDamping(airDrag);
            //}


            //if (rigidbody.GetVelocity().y < 0)
            //{
            //    rigidbody.AddForce(Vector3.Down() * (rigidbody.GetGravityFactor() * fallMultiplier));
            //}
        }

        //if (currentHorizontalVelocity.Length() > aerialMaxMoveSpeed)
        //{
        //    //start applying resitive forces
        //    Vector3 resistiveDirection  = currentHorizontalVelocity;
        //    resistiveDirection.Normalize();
        //    rigidbody.AddForce(-resistiveDirection * aerialResistiveForceFactor);
        //}

    }

    void InitiateDashing()
    {
        UpdateMovementVector();

        SetIframes(true); //set iframes for dashing
        dashTimeElapsed = 0;
        dashCooldownTimer = 0 ;
        rigidbody.SetLinearDamping(0);
        rigidbody.SetVelocityLimits(100000f);
        onDashTrigger = false;

        if (directionVector == Vector3.Zero())
        {
            directionVector = playerOrientation.front;
        }

        desiredSpeed = desiredDashSpeed;

        //target impulse = (targetSpeed - currentSpeed) * mass

        Vector3 desireDashVeclocity = directionVector * desiredDashSpeed;

        Vector3 currentVelocity = rigidbody.GetVelocity();
        currentVelocity.y = 0f;

        Vector3 targetImpulseStrength = desireDashVeclocity - currentVelocity * rigidbody.GetMass();

        rigidbody.AddImpulse(directionVector * targetImpulseStrength.Length());
    }

    void Dashing() 
    { 
        dashTimeElapsed += Time.V_FixedDeltaTime();

        if(dashTimeElapsed > dashDuration)
        {
            playerMoveStates = PlayerMoveStates.AirborneMovement;
            SetIframes(false);
        }
    }

    void InitiateJump()
    {
        jumpsDone++;
        if (jumpsDone >= jumpsBeforeSound)
        {
            audioComponent.PlayRandomSound(jumpVOSFX);
            jumpsDone = 0;
        }
        isLanded = false;
        //rigidbody.AddVelocity(Vector3.Up() * jumpStrength);
        playerMoveStates = PlayerMoveStates.Jumping;

        Vector3 currentVelocity = rigidbody.GetVelocity();
        //currentVelocity.y = 1f * jumpStrength;
        rigidbody.AddImpulse(Vector3.Up() * jumpStrength);
        rigidbody.SetLinearDamping(airDrag);
        rigidbody.SetVelocityLimits(100000); //player movespeed is uncapped in air
        isLerpingGravity = true;
        jumpTimer = 0;
    }

    void SpeedModulator()
    {
        //calculate player current speed
        //get current horizontal velocity
        Vector3 currentHorizontalVelocity = rigidbody.GetVelocity();
        currentHorizontalVelocity.y = 0f;

        if (currentHorizontalVelocity.Length() > desiredSpeed)
        {
            Vector3 resistiveDirection = currentHorizontalVelocity;
            resistiveDirection.Normalize();

            float newSpeed = CalculateSmoothDampVelocityChange(currentHorizontalVelocity.Length(), desiredSpeed, speedChangeSmoothTime, Time.V_FixedDeltaTime());

            float resitiveForce = (rigidbody.GetMass() * (newSpeed - currentHorizontalVelocity.Length())) /Time.V_FixedDeltaTime();

            //Debug.Log("Resitive Force: " + Math.Abs(resitiveForce));
            rigidbody.AddForce(-resistiveDirection * Math.Abs(resitiveForce));
        }

    }

    // ============ INPUT CALLBACK ==========
    private void beginWalkingForward()
    {
        if (!movementIsEnabled) return;
        isMovingForward = true;
    }
    private void endWalkingForward()
    {
        isMovingForward = false;
    }
    private void beginWalkingBackward()
    {
        if (!movementIsEnabled) return;
        isMovingBackward = true;
    }
    private void endWalkingBackward()
    {
        isMovingBackward = false;
    }
    private void beginWalkingLeft()
    {
        if (!movementIsEnabled) return;
        isMovingLeft = true;
    }
    private void endWalkingLeft()
    {
        isMovingLeft = false;
    }
    private void beginWalkingRight()
    {
        if (!movementIsEnabled) return;
        isMovingRight = true;
    }
    private void endWalkingRight()
    {
        isMovingRight = false;
    }

    void triggerJump()
    {
        //check jump conditions
        if (playerMoveStates != PlayerMoveStates.Disabled && playerMoveStates != PlayerMoveStates.Death && jumpEnabled && (jumpTimer > jumpCD))
        {
            playerMoveStates = PlayerMoveStates.StartJump;
        }

    }

    void triggerLandSFX()
    {
        //check if jumped airborne
        if (playerMoveStates != PlayerMoveStates.Disabled && playerMoveStates != PlayerMoveStates.Death && !jumpEnabled && playerMoveStates == PlayerMoveStates.AirborneMovement)
        {
            // audioComponent.PlayRandomSound(landingSFX);
            isLanded = true;
        }

    }

    //dash callback
    void triggerDash()
    {
        if (playerMoveStates != PlayerMoveStates.Disabled 
            && playerMoveStates != PlayerMoveStates.Death 
            && dashCooldownTimer >= dashCooldown 
            && isDashKeyHeld == false 
            && currentStamina >= dashStaminaConsumption)
        {
            audioComponent.PlayRandomSound(dashSFX);
            currentStamina -= dashStaminaConsumption;
            playerMoveStates = PlayerMoveStates.Dashing;
            //rigidbody.SetVelocity(Vector3.Zero());
            onDashTrigger = true;
            isDashKeyHeld = true;
        }
    }

    void dashkeyUpHandler()
    {        
        isDashKeyHeld = false;
    }



    protected override void onCollisionEnter(GameObject other)
    {
        //Debug.Log("Collision Enter: " + other.name);

        Rigidbody_ otherRigidBody = other.getComponent<Rigidbody_>();

        if (otherRigidBody == null) {
            return;
        }

        if(otherRigidBody.GetLayerName() == "Floor")
        {
            contactSurfaces++;
        }


        if (other.tag == "EnemyHitBox")
        {
            EnemyHitBox enemyHitBox = other.getScript<EnemyHitBox>();

            if (enemyHitBox == null || enemyHitBox.HasHitPlayerThisAttack())
                return;
            TakeDamage(enemyHitBox.GetDamage());
            enemyHitBox.OnPlayerHit();
        }

    }


    protected override void onCollisionExit(GameObject other)
    {
        // Debug.Log("Collision Exit: " + other.name);

        Rigidbody_ otherRigidBody = other.getComponent<Rigidbody_>();
        

        if(otherRigidBody == null)
        {

            return;
        }

        if (otherRigidBody.GetLayerName() == "Floor")
        {
            contactSurfaces--;
        }

    }


    // ============ public function==========

    public void TakeDamage(float damage)
    {

        if (isIFrames == true)
        {
            return; //no damage taken
        }

        hitsTaken++;
        if (hitsTaken >= hitsBeforeSound)
        {
            audioComponent.PlayRandomSound(hurtSFX);
            hitsTaken = 0;
        }
        currentHealth = Mathf.Max(0, currentHealth - damage);
        gameUIManager?.SetProgress(GameUIManager.ProgressBarType.HealthBar, currentHealth, maxHealth);
        if (gameUIManager != null)
            gameUIManager.ActivateDamageUI();

        // Placeholder for a player death 
        if (currentHealth <= 0f)
        {
            audioComponent.PlayRandomSound(deathSFX);
            OnPlayerDeath?.Invoke(this, EventArgs.Empty);
        }
    }


    /****************************************************************
    Helper Functions
    ****************************************************************/
    float CalculateSmoothDampVelocityChange(float currentVel,float desiredVel, float smoothTime, float deltaTime)
    {
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;

        // This is the "Magic" formula that stays between 0 and 1
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

        float velError = currentVel - desiredVel;
        float temp = (velError + (omega * velError * deltaTime)) * deltaTime;

        // Note: We subtract temp and multiply by our decay factor (exp)
        float targetVel = desiredVel + (velError - temp) * exp;

        return targetVel; // Return the actual speed we want to reach next frame
    }

    float GetCurrentHorizontalVelocity()
    {
        Vector3 currentVelocity = rigidbody.GetVelocity();
        currentVelocity.y = 0f;
        return currentVelocity.Length();
    }

    public Vector3 GetDirectionVector() //for movement controller
    {
        return directionVector;
    }

    public void SetIframes(bool value)
    {
        if (value == true)
        {
            isIFrames = value;
            rigidbody.SetPhysicsLayer("PlayerGhost");
        }
        else 
        {
            isIFrames = value;
            rigidbody.SetPhysicsLayer("Moving");
        }
    }

    //Sound function
    private void HandleFootstepSound()
    {
        if (isGrounded && IsMoving())
        {
            timeSinceLastFootstep += Time.V_FixedDeltaTime();
            if (timeSinceLastFootstep >= timeBetweenSteps)
            {
                audioComponent.PlayRandomSound(footstepSFX);
                timeSinceLastFootstep = 0;
            }
        }
    }


    private bool IsMoving() => isMovingBackward || isMovingForward || isMovingLeft || isMovingRight;
    /****************************************************************
    Public Functions
    ****************************************************************/

    public void GravityFreeze(bool value)
    {
        if (value)
        {
            isLerpingGravity = false;
            rigidbody.SetGravityFactor(0f);
        }
        else
        {
            isLerpingGravity = true;
            rigidbody.SetGravityFactor(float.Lerp(jumpGravityFactor, baseGravityFactor, gravityLerpTimer / lerpGravityFactorDuration));
        }


    }

    public void GainHealth(float heal)
    {
        currentHealth += heal;
        currentHealth = Mathf.Min(maxHealth, currentHealth);
    }

    public void PositionFreeze(bool value)
    {
        if (value)
        {
            restoreVelocity = rigidbody.GetVelocity();
            rigidbody.SetVelocity(Vector3.Zero());
            directionVector = Vector3.Zero();
        }
        //we might want to conserve momentum after unfreezing so there is this possibility of a boolean
        else if (!value)
        {
            rigidbody.SetVelocity(restoreVelocity);
        }
    }


    public void Reset()
    {
        isMovingBackward = isMovingForward = isMovingLeft = isMovingRight = false;
        rigidbody.SetVelocity(Vector3.Zero());
        currentHealth = maxHealth;
        gameUIManager?.SetProgress(GameUIManager.ProgressBarType.HealthBar, currentHealth, maxHealth);
    }

}


public enum PlayerMoveStates 
{
    Disabled,
    InitState,
    GroundedMovement,
    AirborneMovement,
    StartJump,
    Jumping,
    Dashing,
    Death,

}