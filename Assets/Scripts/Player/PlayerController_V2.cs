// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerController_V2 : Script
{
    // ==================================
    // Parameters
    // ==================================
    [SerializableField]
    public float groundAcceleration = 2f; //rate of change of player speed
    [SerializableField]
    public float groundMaxMoveSpeed = 10f; //the max speed/ the desirable speed of the player on ground
    [SerializableField]
    public float groundDrag = 20.0f; //deceleration
    [SerializableField]
    public float groundDetectionRayCast = 3.0f; //raycast detection distance to ground. ideally to crosscheck with player height. (Not this is not use for airborne/ground movement checking)
    [SerializableField]
    public float airAcceleration = 2f; //rate of change of player speed
    [SerializableField]
    public float aerialMaxMoveSpeed = 8f; //the max speed/ the desirable speed of the player in air
    //[SerializableField]
    //public float aerialResistiveForceFactor = 1f; //resistive force when the player is moving too fast
    [SerializableField]
    public float airDrag = 0f; //lost of speed per second when in the air, note this should be zero at all times??? otherwise this messes up gravity as well, so player falls slower than intended
    [SerializableField]
    public float jumpStrength =   10f;
    [SerializableField]
    public float jumpGravityFactor = 1f; //should be =< than base gravity factor
    [SerializableField]
    public float lerpGravityFactorDuration = 1f;
    //[SerializableField]
    //public float dashStrength = 10f;
    [SerializableField]
    public float desiredDashSpeed = 100f;
    [SerializableField]
    public float dashDuration = 1f;
    [SerializableField]
    public float dashCooldown = 0.5f;
    [SerializableField]
    public float speedChangeSmoothTime = 0.1f;
    [SerializableField]
    public float baseGravityFactor = 1f; //gravity factor for groundbased movements

    [SerializableField] //health
    public float maxHealth = 100f;

    // ==================================
    // References
    // ==================================
    [SerializableField]
    private Transform_? playerOrientation = null;
    [SerializableField]
    private GameUIManager? gameUIManager = null;
    private Transform_? transform;
    private Rigidbody_? rigidbody;
    private AudioComponent_? audioComponent;

    // ==================================
    // Runtime Variables
    // ==================================
    // WASD
    private PlayerMoveStates playerMoveStates = PlayerMoveStates.Disabled;
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
    private float jumpCD = 0.5f;
    private int contactSurfaces = 0;

    private Vector3 directionVector = Vector3.Zero();
    private float   desiredSpeed = 0.0f; //the speed of the player the player should move at.

    //Timers
    private float jumpTimer = 0.0f;
    private float dashCooldownTimer = 0.0f;
    private float gravityLerpTimer = 0.0f;
    private float dashTimeElapsed = 0.0f;

    // ==================================
    // Player Stats
    // ==================================
    private float currentHealth = 0f;


    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        playerMoveStates = PlayerMoveStates.InitState;
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();
        audioComponent = getComponent<AudioComponent_>();

        //TBH should get other system to check for lockedmouse
        CameraAPI.LockMouse();

        // Health
        currentHealth = maxHealth;

        rigidbody.SetGravityFactor(baseGravityFactor);
        

        //Input initialisation
        MapKey(Key.W, beginWalkingForward, endWalkingForward);
        MapKey(Key.A, beginWalkingLeft, endWalkingLeft);
        MapKey(Key.S, beginWalkingBackward, endWalkingBackward);
        MapKey(Key.D, beginWalkingRight, endWalkingRight);

        MapKey(Key.Space, triggerJump);

        MapKey(Key.LeftShift, triggerDash);

    }

    // This function is invoked every update.
    protected override void update()
    {

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        //Add to timer
        jumpTimer += Time.V_FixedDeltaTime();
        dashCooldownTimer += Time.V_FixedDeltaTime();

        Debug.Log(GetCurrentHorizontalVelocity());


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
            PlayerMoveStates.Jumping != playerMoveStates   &&
            PlayerMoveStates.Dashing != playerMoveStates)
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
            case PlayerMoveStates.Jumping:
                {
                    InitiateJump();
                }
                break;
            case PlayerMoveStates.Death:
                {

                }
                break;



        }



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
            playerMoveStates = PlayerMoveStates.GroundedMovement;
            rigidbody.SetLinearDamping(groundDrag);
            rigidbody.SetVelocityLimits(groundMaxMoveSpeed);
            jumpEnabled = true;
           // Debug.Log("Grounded");

            //if stil in lerping gravityfactor change gravity to jump gravity now
            //if (isLerpingGravity)
            //{
            //    rigidbody.SetGravityFactor(baseGravityFactor);
            //    isLerpingGravity = false;
            //    gravityLerpTimer = 0f;
            //}



        }
        else
        {
            playerMoveStates = PlayerMoveStates.AirborneMovement;
            rigidbody.SetLinearDamping(airDrag);
            rigidbody.SetVelocityLimits(100000); //player movespeed is uncapped in air
            jumpEnabled = false;
           // Debug.Log("Airborne");
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

            //player is moving faster than allowed, stop accelerating
            if (currentHorizontalVelocity.Length() < aerialMaxMoveSpeed)
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

        dashTimeElapsed = 0;
        rigidbody.SetLinearDamping(0);
        rigidbody.SetVelocityLimits(100000f);
        onDashTrigger = false;

        if (directionVector == Vector3.Zero())
        {
            directionVector = playerOrientation.front;
        }

        desiredSpeed = desiredDashSpeed;

        //target impulse = (targetSpeed - currentSpeed) * mass
        float targetImpulseStrength = desiredDashSpeed - GetCurrentHorizontalVelocity() * rigidbody.GetMass();

        rigidbody.AddImpulse(directionVector * targetImpulseStrength);
    }

    void Dashing() 
    { 
        dashTimeElapsed += Time.V_FixedDeltaTime();

        if(dashTimeElapsed > dashDuration)
        {
            playerMoveStates = PlayerMoveStates.AirborneMovement;
        }

    
    
    }

    void InitiateJump()
    {

        //rigidbody.AddVelocity(Vector3.Up() * jumpStrength);
        playerMoveStates = PlayerMoveStates.AirborneMovement;

        Vector3 currentVelocity = rigidbody.GetVelocity();
        currentVelocity.y = 1f * jumpStrength;
        rigidbody.SetVelocity(currentVelocity);

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

            Debug.Log("Resitive Force: " + Math.Abs(resitiveForce));
            rigidbody.AddForce(-resistiveDirection * Math.Abs(resitiveForce));
        }

    }

    // ============ INPUT CALLBACK ==========
    private void beginWalkingForward()
    {
        isMovingForward = true;
    }
    private void endWalkingForward()
    {
        isMovingForward = false;
    }
    private void beginWalkingBackward()
    {
        isMovingBackward = true;
    }
    private void endWalkingBackward()
    {
        isMovingBackward = false;
    }
    private void beginWalkingLeft()
    {
        isMovingLeft = true;
    }
    private void endWalkingLeft()
    {
        isMovingLeft = false;
    }
    private void beginWalkingRight()
    {
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
            playerMoveStates = PlayerMoveStates.Jumping;
        }

    }

    //dash callback
    void triggerDash()
    {
        if (playerMoveStates != PlayerMoveStates.Disabled && playerMoveStates != PlayerMoveStates.Death && dashCooldownTimer > dashCooldown)
        {
            playerMoveStates = PlayerMoveStates.Dashing;
            //rigidbody.SetVelocity(Vector3.Zero());
            onDashTrigger = true;
        }
    }


    protected override void onCollisionEnter(GameObject other)
    {
        //Debug.Log("Collision Enter: " + other.name);

        Rigidbody_ otherRigidBody = other.getComponent<Rigidbody_>();

        if (otherRigidBody == null) {
            return;
        }

        if(otherRigidBody.GetLayerName() == "Floor" || otherRigidBody.GetLayerName() == "NonMoving")
        {
            contactSurfaces++;
        }


        //if ( == null)
        //{
        //    otherRigidBody.GetLayerName()
        //}

        //Debug.Log("Layer Exit: " + otherRigidBody.GetLayerName());
        //otherRigidBody.GetLayerName();

    }


    protected override void onCollisionExit(GameObject other)
    {
        // Debug.Log("Collision Exit: " + other.name);

        Rigidbody_ otherRigidBody = other.getComponent<Rigidbody_>();

        if(otherRigidBody == null)
        {
            return;
        }

        if (otherRigidBody.GetLayerName() == "Floor" || otherRigidBody.GetLayerName() == "NonMoving")
        {
            contactSurfaces--;
        }

    }


    //Helper function
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

}


enum PlayerMoveStates 
{
    Disabled,
    InitState,
    GroundedMovement,
    AirborneMovement,
    Jumping,
    Dashing,
    Death,

}