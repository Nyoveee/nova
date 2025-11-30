// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerRotateController : Script
{

    public float cameraSensitivity = 0.1f;
    [SerializableField]
    private Transform_? playerOrientation = null; //Movement (XYZ) handled by this script + inheritence. Camera rotation is handled by PlayerRotateController which in unaffected by inheritence 

    [SerializableField]
    private Transform_? playerCameraPos = null; //Camera rotation is handled by PlayerRotateController which in unaffected by inheritence 
    [SerializableField]
    private Transform_  playerBody;

    [SerializableField]
    private PlayerController playerController;
    //private float rotationX = 0;
    //private float rotationY = 0;
    [SerializableField]
    public bool lockOutRotate = false;

    private float accumulateDifferenceX = 0;
    private float accumulateDifferenceY = 0;
    private float height = 0;
    private Vector3 velocityMod = Vector3.Zero();
    // This function is first invoked when game starts.
    protected override void init()
    {
        // Controls..
        MouseMoveCallback(CameraMovement);
        accumulateDifferenceX = 0;
        accumulateDifferenceY = 0;

        height = gameObject.transform.position.y - playerBody.position.y;
        Time.timeScale = 1f;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        
        UpdateCamera();



        //MoveToOrientation();
        Invoke(MoveToOrientation, 0);
        //gameObject.transform.position = playerCameraPos.position;
        //UpdateCamera();
        //Invoke(MoveToOrientation,0);

    }

    protected override void fixedUpdate()
    {




    }

     

    private void MoveToOrientation()
    {
        //gameObject.transform.position = playerCameraPos.position;
        if (lockOutRotate == true)
        {
            lockOutRotate = false;
            return;

        }

        //playerCameraPos.localPosition = position;
        //playerCameraPos.localEulerAngles = gameObject.transform.localEulerAngles;
        //gameObject.transform.localPosition = playerCameraPos.localPosition;

        //Vector3 position = playerBody.localPosition;
        //position.y += height;
        //gameObject.transform.localPosition = position;
        //===========================================================
        Vector3 position = playerBody.localPosition; //after intergration step transform of playerbody


        //Get the lerp factor
        float t = Time.V_AccumulatedTime() / Time.V_FixedDeltaTime();

        t = Mathf.Min(1, t);

        Vector3 smoothPos = Vector3.Lerp(playerController.lastFDTTransform.localPosition, position, t);

        smoothPos.y += height;
        gameObject.transform.localPosition = smoothPos;

        //==========================================================================
        //Vector3 position = playerBody.position;
        //Vector3 smoothPos = SmoothDamp(gameObject.transform.position,position,ref velocityMod,0.1f,Time.V_DeltaTime());

        //smoothPos.y += height;
        //gameObject.transform.position = smoothPos;

    }

    //private void CameraMovement(float deltaMouseX, float deltaMouseY)
    //{
    //    // We rotate our parent in the y axis..

    //    // x is pitch, y is yaw
    //    Vector3 euler = gameObject.transform.localEulerAngles;

    //    euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
    //    euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
    //    euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);


    //    playerOrientation.localEulerAngles = new Vector3(0, euler.y, 0);

    //    gameObject.transform.localEulerAngles = euler;

    //}

    private void UpdateCamera()
    {
        if (lockOutRotate == true)
        { 
           // lockOutRotate = false;
            return;
        
        }
        Vector3 euler = gameObject.transform.localEulerAngles;

        euler.x -= cameraSensitivity * accumulateDifferenceY * Time.V_DeltaTime();
        euler.y -= cameraSensitivity * accumulateDifferenceX * Time.V_DeltaTime();
        euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
        playerOrientation.localEulerAngles = new Vector3(0, euler.y, 0);

        gameObject.transform.localEulerAngles = euler;


        accumulateDifferenceX = 0;
        accumulateDifferenceY = 0;
    }


    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        accumulateDifferenceX += deltaMouseX;
        accumulateDifferenceY += deltaMouseY;

        //if (Time.V_AccumulatedTime() < Time.V_FixedDeltaTime())
        //{

            //Vector3 euler = gameObject.transform.localEulerAngles;

            //euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
            //euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
            //euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
            //playerBody.localEulerAngles = new Vector3(0, euler.y, 0);

            //gameObject.transform.localEulerAngles = euler;
            //lockOutRotate = true;
        //}
        //else
        //{
        //    lockOutRotate = false;
        
        //}


        //accumulateDifferenceX = 0;
        //accumulateDifferenceY = 0;


        //Vector3 euler = gameObject.transform.eulerAngles;

        //euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
        //euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
        //euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
        //playerOrientation.eulerAngles = new Vector3(0, euler.y, 0);

        //gameObject.transform.eulerAngles = euler;


    }

    //Vector3 SmoothDamp(
    //Vector3 current,
    //Vector3 target,
    //ref Vector3 currentVelocity,
    //float smoothTime,
    //float deltaTime,
    //float maxSpeed = 10000f
    //)
    //{
    //    // Safety
    //    //smoothTime = Math.Max(0.0001f, smoothTime);

    //    float omega = 2.0f / smoothTime;

    //    float x = omega * deltaTime;
    //    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

    //    Vector3 change = current - target;
    //    Vector3 originalTo = target;

    //    // Clamp maximum speed
    //    float maxChange = maxSpeed * smoothTime;
    //    float maxChangeSq = maxChange * maxChange;
    //    float magSq = change.Length() * change.Length();

    //    if (magSq > maxChangeSq)
    //    {
    //        float mag = Mathf.Sqrt(magSq);
    //        change = change * (maxChange / mag);
    //    }

    //    target = current - change;

    //    Vector3 temp = (currentVelocity + omega * change) * deltaTime;
    //    currentVelocity = (currentVelocity - omega * temp) * exp;

    //    Vector3 output = target + (change + temp) * exp;

    //    // Prevent overshoot
    //    Vector3 origMinusCurrent = originalTo - current;
    //    Vector3 outMinusOrig = output - originalTo;

    //    if (Vector3.Dot(origMinusCurrent, outMinusOrig) > 0.0f)
    //    {
    //        output = originalTo;
    //        currentVelocity = Vector3.Zero();
    //    }

    //    return output;
    //}


    //public static float SmoothDamp(float current , float target, ref float currentVelocity, float smoothTime, float m ,)
    //{ 
    
    
    
    
    //}

}