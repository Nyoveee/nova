// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerController_V2 : Script
{
    // ==================================
    // Parameters
    // ==================================
    [SerializableField]
    public float forwardMoveSpeed = 10f;
    [SerializableField]
    public float strafingMoveSpeed = 8f;
    [SerializableField]
    public float backwardMoveSpeed = 8f;
    [SerializableField]
    public float drag = 20.0f;
    [SerializableField]
    public float aerialMoveSpeed = 8f;
    [SerializableField]
    public float jumpStrength = 5f;
    [SerializableField]
    public float fallMultiplier = 5f;
    [SerializableField]
    public float dashStrength = 10f;
    [SerializableField]
    public float dashDuration = 1f;
    [SerializableField]
    public float dashCooldown = 0.5f;

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
    private bool isGrounded = false;
    private bool wasGrounded = false;
    private int  jumpCount = 0;

    private Vector3 directionVector = Vector3.Zero();

    //health
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



    }

    // This function is invoked every update.
    protected override void update()
    {
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        if (PlayerMoveStates.InitState != playerMoveStates)
        {
            CheckMovementTypeState();
        }


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

                }
                break;
            case PlayerMoveStates.Dashing:
                {

                }
                break;
            case PlayerMoveStates.Jumping:
                {

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


        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Floor" };
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), 1f, mask);

        if (result != null)
        {
            isGrounded = true;
        }
        else
        { 
            isGrounded = false;
        }

        if (isGrounded == true && wasGrounded == false)
        { 
            //player as landed on the ground
            playerMoveStates = PlayerMoveStates.GroundedMovement;
            jumpCount = 1;
        }

        if (isGrounded == false && wasGrounded == true)
        { 
            //player has just left the ground

        }

    }

    void InitMovement()
    {
        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Floor" };
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down(), 1f, mask);

        if (result != null)
        { 
            isGrounded = true;
            wasGrounded = true;
            playerMoveStates = PlayerMoveStates.GroundedMovement;
            jumpCount = 1;
        }

    }


    void UpdateMovementVector() 
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

        if (isMoving == false)
        { 
            directionVector = Vector3.Zero();
        }
    }


    void UpdateGroundedMovement() 
    {

        UpdateMovementVector();




        if (isMovingForward == true)
        { 
            rigidbody.AddForce(directionVector * forwardMoveSpeed);
        }
        else if(isMovingBackward == true)
        {
            rigidbody.AddForce(directionVector * backwardMoveSpeed);
        }
        else if(isMovingLeft || isMovingRight == true)
        {
            rigidbody.AddForce(directionVector * strafingMoveSpeed);
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