// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class PlayerController : Script
{

    // Move speed
    public float maximumMoveSpeed = 5f;
    public float accelerationStrength = 20f;

    // Jump
    public float jumpStrength = 100f;
    public int maxJumpCount = 2;
    private int jumpsBeforeSound = 5;
    private int jumpsDone = 0;

    // Hurt
    private int hitsBeforeSound = 3;
    private int hitsTaken = 0;

    // Dash
    public float dashDuration = 1f;
    public int dashCount = 3;
    public float dashCooldown = 3f; // per dash, you have to wait `dashCooldown` seconds.
    public float dashStrength = 2f;

    // Health
    public float maxHealth = 100f;

    [SerializableField]
    private Transform_? playerOrientation = null; //Movement (XYZ) handled by this script + inheritence. Camera rotation is handled by PlayerRotateController which in unaffected by inheritence 

    // ==================================
    // Internal / Runtime variables..
    // ==================================
    private GameUIManager gameUIManager;
    private Transform_? transform;
    private Rigidbody_? rigidbody;
    private AudioComponent_? audioComponent;

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
    private float timeBetweenSteps = 0.36f;
    private float timeSinceLastFootstep = 0f;

    // WASD
    private bool isMovingForward = false;
    private bool isMovingBackward = false;
    private bool isMovingLeft = false;
    private bool isMovingRight = false;
    private bool isGrounded = false;

    // Jump..
    private bool wasInMidAir = false;
    int jumpCount = 0;

    // Dash
    private float dashTimerCap;
    private float dashTimer;
    private bool isDashing = false;
    private float dashTimeElapsed = 0f;
    private Vector3 dashVector;

    // Health
    private float currentHealth = 0f;

    private bool enabled = true;
    public bool ToEnable { get { return enabled; } set { enabled = value; } }

    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        audioComponent = getComponent<AudioComponent_>();

        CameraAPI.LockMouse();

        // Health
        currentHealth = maxHealth;

        // Dash
        dashTimerCap = dashCooldown * dashCount;
        dashTimer = dashTimerCap;



        MapKey(Key.W, beginWalkingForward, endWalkingForward);
        MapKey(Key.A, beginWalkingLeft, endWalkingLeft);
        MapKey(Key.S, beginWalkingBackward, endWalkingBackward);
        MapKey(Key.D, beginWalkingRight, endWalkingRight);

        MapKey(Key.Space, jump);

        MapKey(Key.LeftShift, dashInputHandler);
    }

    // This function is invoked every fixed update.
    protected override void fixedUpdate()
    {
        // ===================================
        // Check if its grounded..
        // ===================================
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), 10f, gameObject);
        if (result != null)
        {
            isGrounded = true;

            // this branch is only executed once, per landing..
            if (wasInMidAir)
            {

                wasInMidAir = false;
                // @TODO: play landing sound..
            
            }
        }
        else
        {
            isGrounded = false;
            wasInMidAir = true;
        }

        if (!enabled)
        {
            return;
        }

        // ===================================
        // Dash handling
        // ===================================
        if (isDashing)
        {
            handleDashing();
        }
        // ===================================
        // Movement handling..
        // ===================================
        else
        {
            dashTimer = Mathf.Clamp(dashTimer + Time.V_FixedDeltaTime(), 0f, dashTimerCap);
            handleMovement();
        }
        // ===================================
        // UI
        // ===================================
        if (gameUIManager != null)
        {
            gameUIManager.SetProgress(GameUIManager.ProgressBarType.DashBar, dashTimer, dashTimerCap);
            gameUIManager.SetProgress(GameUIManager.ProgressBarType.HealthBar, currentHealth, maxHealth);
            // gameUIManager.SetHealthText((int)currentHealth);
        }

    }

    protected override void update()
    {

        //// ==============================
        //// Handles WASD movement, we calculate the oriented directional vector from input..
        //// ==============================
        //Vector3 orientedFront = new Vector3(playerOrientation.front.x, 0, playerOrientation.front.z);
        //Vector3 orientedRight = new Vector3(playerOrientation.right.x, 0, playerOrientation.right.z);

        //Vector3 directionVector = Vector3.Zero();

        //orientedFront.Normalize();
        //orientedRight.Normalize();

        //// We accumulate all directional input command..
        //bool isMoving = false;

        //if (isMovingForward)
        //{
        //    directionVector += orientedFront;
        //    isMoving = true;
        //}
        //if (isMovingBackward)
        //{
        //    directionVector -= orientedFront;
        //    isMoving = true;

        //}
        //if (isMovingLeft)
        //{
        //    directionVector -= orientedRight;
        //    isMoving = true;

        //}
        //if (isMovingRight)
        //{
        //    directionVector += orientedRight;
        //    isMoving = true;
        //}

        //if (isMoving)
        //{
        //    directionVector.Normalize();
        //    Vector3 movingVector = directionVector * maximumMoveSpeed * Time.V_DeltaTime();

        //    transform.position += movingVector;

        //}
        //// Stop user from moving if grounded, in the case where user has not pressed any key..
        //else if (isGrounded)
        //{
        //    // rigidbody.SetVelocity(new Vector3(0f, rigidbody.GetVelocity().y, 0f));
        //}


    }

    public void GainHealth(float heal)
    {
        currentHealth += heal;
        currentHealth = Mathf.Min(maxHealth, currentHealth);
    }

    /***********************************************************
       Public Functions
    ***********************************************************/
    public void TakeDamage(float damage)
    {
        hitsTaken++;
        if(hitsTaken >= hitsBeforeSound)
        {
            audioComponent.PlayRandomSound(hurtSFX);
            hitsTaken = 0;
        }
        currentHealth = Mathf.Max(0, currentHealth - damage);
        //Debug.Log("Player health: " + currentHealth);
        if (gameUIManager != null)
            gameUIManager.ActivateDamageUI();

        // Placeholder for a player death 
        if (currentHealth <= 0f)
        {
            audioComponent.PlayRandomSound(deathSFX);
        }
    }
    /***********************************************************
       Movement
    ***********************************************************/
    private void handleMovement()
    {
        // ==============================
        // Handles WASD movement, we calculate the oriented directional vector from input..
        // ==============================
        Vector3 orientedFront = new Vector3(playerOrientation.front.x, 0, playerOrientation.front.z);
        Vector3 orientedRight = new Vector3(playerOrientation.right.x, 0, playerOrientation.right.z);

        Vector3 directionVector = Vector3.Zero();

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

        // ==============================
        // We calculate resulting velocity based on input..
        // ==============================

        // This frame, user has at least pressed one movement key..
        if (isMoving)
        {
            directionVector.Normalize();
            Vector3 movingVector = directionVector * maximumMoveSpeed;

            if (isGrounded)
            {
                Vector3 newVelocity = new Vector3(movingVector.x, rigidbody.GetVelocity().y, movingVector.z);
                rigidbody.SetVelocity(newVelocity);
                HandleFootstepSound();
            }
            else
            {
                Vector3 forceVector = new Vector3(movingVector.x, 0f, movingVector.z);
                rigidbody.AddForce(forceVector * accelerationStrength);
            }

            // Speed consistency.. we clamp it's speed based on some max move speed.. (mid air)
            Vector3 flatVelocity = new Vector3(rigidbody.GetVelocity().x, 0f, rigidbody.GetVelocity().z);
            float length = flatVelocity.Length();

            if (length > maximumMoveSpeed)
            {
                flatVelocity.Normalize();
                Vector3 clampedVelocity = flatVelocity * maximumMoveSpeed;
                rigidbody.SetVelocity(new Vector3(clampedVelocity.x, rigidbody.GetVelocity().y, clampedVelocity.z));
            }
        }
        // Stop user from moving if grounded, in the case where user has not pressed any key..
        else if (isGrounded)
        {
            rigidbody.SetVelocity(new Vector3(0f, rigidbody.GetVelocity().y, 0f));
        }
    }

    private void handleDashing()
    {
        // We finished dashing..
        if (dashTimeElapsed > dashDuration)
        {
            isDashing = false;
            rigidbody.SetVelocity(Vector3.Zero());
            return;
        }

        // constantly apply velocity.
        // rigidbody.SetVelocity(dashVector * dashStrength);
        //rigidbody.AddImpulse(dashVector * dashStrength);
        dashTimeElapsed += Time.V_FixedDeltaTime();
    }

    private void HandleFootstepSound()
    {
        if (isGrounded && rigidbody.GetVelocity != Vector3.Zero)
        {
            timeSinceLastFootstep += Time.V_FixedDeltaTime();
            if (timeSinceLastFootstep >= timeBetweenSteps)
            {
                audioComponent.PlayRandomSound(footstepSFX);
                timeSinceLastFootstep = 0;
            }
        }
    }
    //private void CameraMovement(float deltaMouseX, float deltaMouseY)
    //{
    //    // We rotate our parent in the y axis..
    //    // transform.rotate(Vector3.Up, deltaMouseX * cameraSensitivity);
    //    //transform.rotate(Vector3.Front, deltaMouseY * cameraSensitivity);

    //    // x is pitch, y is yaw
    //    Vector3 euler = cameraObject.localEulerAngles;

    //    euler.x -= cameraSensitivity * deltaMouseY;
    //    euler.y -= cameraSensitivity * deltaMouseX;
    //    euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
    //    // euler.y = Mathf.Clamp(euler.y, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);

    //    cameraObject.localEulerAngles = euler;  
    //}

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

    private void jump()
    {
        if (!enabled)
        {
            return;
        }

        if (isGrounded && !isDashing)
        {
            jumpsDone++;
            if(jumpsDone >= jumpsBeforeSound)
            {
                audioComponent.PlayRandomSound(jumpVOSFX);
                jumpsDone = 0;
            }
            Vector3 currentVelocity = rigidbody.GetVelocity();
            currentVelocity.y = 1f * jumpStrength;
            rigidbody.SetVelocity(currentVelocity);
        }
    }

    private void dashInputHandler()
    {
        if (!enabled || isDashing || dashTimer < dashCooldown)
        {
            return;
        }


        // We initialise dashing mechanic..
        isDashing = true;
        dashTimer -= dashCooldown;
        dashTimeElapsed = 0f;
        audioComponent.PlayRandomSound(dashSFX);

        // Determine dashing vector..
        if (isMovingBackward)
        {
            dashVector = new Vector3(-playerOrientation.front.x, 0, -playerOrientation.front.z);
        }
        else if (isMovingLeft && !isMovingRight)
        {
            dashVector = new Vector3(-playerOrientation.right.x, 0, -playerOrientation.right.z);
        }
        else if (isMovingRight && !isMovingLeft)
        {
            dashVector = new Vector3(playerOrientation.right.x, 0, playerOrientation.right.z);
        }
        else
        {
            // Default forward..
            dashVector = new Vector3(playerOrientation.front.x, 0, playerOrientation.front.z);
        }

        dashVector.Normalize();
        //rigidbody.SetVelocity(dashVector * dashStrength);
        rigidbody.AddImpulse(dashVector * dashStrength);
    }

    /***********************************************************
        Collision Events
    ***********************************************************/
    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "EnemyHitBox")
        {
            EnemyHitBox enemyHitBox = other.getScript<EnemyHitBox>();

            if (enemyHitBox == null || enemyHitBox.HasHitPlayerThisAttack())
                return;

            TakeDamage(enemyHitBox.GetDamage());
            enemyHitBox.OnPlayerHit();
        }
    }
}