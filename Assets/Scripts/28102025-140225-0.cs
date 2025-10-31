// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerController : Script
{
    public float cameraSensitivity = 0.03f;
    
    [SerializableField]
    private Transform_? cameraObject = null;

    private Rigidbody_? rigidbody;

    private bool isMovingForward = false;

    // This function is first invoked when game starts.
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();

        CameraAPI.LockMouse();

        Input.MouseMoveCallback(CameraMovement);
        Input.MapKey(Key.W, beginWalkingForward, endWalkingForward);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
#if false
        if (rigidbody != null && cameraObject!= null && isMovingForward)
        {
            rigidbody.addImpulse(cameraObject.front);
        }
#endif
    }

    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        if (cameraObject != null)
        {
            // x is pitch, y is yaw
            Vector3 euler = cameraObject.localEulerAngles;

            euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
            euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
            euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
            euler.y = Mathf.Clamp(euler.y, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);

            cameraObject.localEulerAngles = euler;  
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
}