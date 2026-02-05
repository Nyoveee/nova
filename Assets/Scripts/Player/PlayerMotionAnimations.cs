// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerMotionAnimations : Script
{

    // ===========================================
    // References
    // ===========================================
    public PlayerWeaponController playerWeaponController;


    // ===========================================
    // Inspector variables
    // ===========================================

    public float motionChangeDuration = 0.1f;
    public float gunMotionOffsetAmount = 2f;

    private Vector3 directionVector = Vector3.Zero();
    private Vector3 currentOffset = Vector3.Zero();

    private Vector3 movementVelocity = Vector3.Zero();
    private Rigidbody_? rigidbody;
    // ===========================================
    // Runtime variables
    // ===========================================
    private Transform_ gunPosition;
    private Transform_ throwPosition;
    private Transform_ gunHolder;

    private Vector3 gunHolderBasePosition;
    private Vector3 throwPositionBasePosition;
    private Vector3 gunPositionBasePosition;
    
    //Movement Vector
    private bool isMovingForward = false;
    private bool isMovingBackward = false;
    private bool isMovingLeft = false;
    private bool isMovingRight = false;


    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {

        rigidbody = getComponent<Rigidbody_>();
        gunHolder = playerWeaponController.gunHolder;
        throwPosition = playerWeaponController.throwPosition;
        gunPosition = playerWeaponController.gunPosition;

        gunHolderBasePosition = gunHolder.localPosition;
        throwPositionBasePosition = throwPosition.localPosition;
        gunPositionBasePosition = gunPosition.localPosition;

        MapKey(Key.W, beginWalkingForward, endWalkingForward);
        MapKey(Key.A, beginWalkingLeft, endWalkingLeft);
        MapKey(Key.S, beginWalkingBackward, endWalkingBackward);
        MapKey(Key.D, beginWalkingRight, endWalkingRight);



    }

    // This function is invoked every update.
    protected override void update()
    {

        //currentOffset = CalculateSmoothDampPositionChange(currentOffset, gunMotionOffsetAmount * desiredVelocty, motionChangeDuration, Time.V_DeltaTime());

        //gunPosition.localPosition = gunPositionBasePosition + currentOffset;
        //throwPosition.localPosition = throwPositionBasePosition + currentOffset;

        //Vector3 newtestposition = gunHolder.localPosition;

        // newtestposition.z += desiredVelocty.z;

        //gunHolder.localPosition = newtestposition;
        //Debug.Log(desiredVelocty);
        //gunHolder.localPosition = 0.1f * Time.V_DeltaTime();
        //gunHolder.localPosition += ;


        UpdateMotionVector();
        Debug.Log("Y velcoity: "+ rigidbody.GetVelocity().y);
        currentOffset = CalculateSmoothDampPositionChange(currentOffset, gunMotionOffsetAmount * -directionVector, motionChangeDuration, Time.V_DeltaTime());


        gunPosition.localPosition = gunPositionBasePosition + currentOffset;
        throwPosition.localPosition = throwPositionBasePosition + currentOffset;


    }


    //public void SetGunMotionChange(Vector3 desiredVelocity)
    //{
    //    this.movementVelocity = desiredVelocity;
    //}

    Vector3 CalculateSmoothDampPositionChange(Vector3 currentPosition, Vector3 desiredPosition, float smoothTime, float deltaTime)
    {
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;

        // This is the "Magic" formula that stays between 0 and 1
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

        Vector3 velError = currentPosition - desiredPosition;
        Vector3 temp = (velError + (omega * velError * deltaTime)) * deltaTime;

        // Note: We subtract temp and multiply by our decay factor (exp)
        Vector3 targetVel = desiredPosition + (velError - temp) * exp;

        return targetVel; // Return the position
    }

    void UpdateMotionVector()
    {
        // ==============================
        // Handles WASD movement, we calculate the oriented directional vector from input..
        // ==============================
        Vector3 orientedFront = Vector3.Front();
        Vector3 orientedRight = Vector3.Left();

        directionVector = Vector3.Zero();

        //orientedFront.Normalize();
        //orientedRight.Normalize();

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

        if (rigidbody.GetVelocity().y > 0.1f)
        {
            directionVector.y = 1f;
        }
        else if (rigidbody.GetVelocity().y < -0.1f)
        {
            directionVector.y = -1f;
        }
        else
        {
            directionVector.y = 0;
        }

        directionVector.Normalize();
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



    //// This function is invoked every update.
    //protected override void fixedUpdate()
    //{}

    //// This function is invoked when destroyed.
    //protected override void exit()
    //{}

}