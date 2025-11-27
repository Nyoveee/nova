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

    //private float rotationX = 0;
    //private float rotationY = 0;

    private float accumulateDifferenceX = 0;
    private float accumulateDifferenceY = 0;

    // This function is first invoked when game starts.
    protected override void init()
    {
        // Controls..
        MouseMoveCallback(CameraMovement);
        accumulateDifferenceX = 0;
        accumulateDifferenceY = 0;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        UpdateCamera();
        //MoveToOrientation();
        Invoke(MoveToOrientation, 0);
        //UpdateCamera();
        //Invoke(MoveToOrientation,0);

    }

    protected override void fixedUpdate()
    {




    }

     

    private void MoveToOrientation()
    {
        gameObject.transform.position = playerCameraPos.position;
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
        Vector3 euler = gameObject.transform.eulerAngles;

        euler.x -= cameraSensitivity * accumulateDifferenceY * Time.V_DeltaTime();
        euler.y -= cameraSensitivity * accumulateDifferenceX * Time.V_DeltaTime();
        euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
        playerOrientation.localEulerAngles = new Vector3(0, euler.y, 0);

        gameObject.transform.eulerAngles = euler;


        accumulateDifferenceX = 0;
        accumulateDifferenceY = 0;
    }


    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        accumulateDifferenceX += deltaMouseX;
        accumulateDifferenceY += deltaMouseY;

    }
}