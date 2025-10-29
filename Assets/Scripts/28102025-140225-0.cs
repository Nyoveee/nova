// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerController : Script
{
    public float cameraSensitivity = 0.03f;
    
    [SerializableField]
    private Transform_? cameraObject = null;
    // This function is first invoked when game starts.
    protected override void init()
    {
        CameraAPI.LockMouse();
        Input.MouseMoveCallback(CameraMovement);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        if (cameraObject != null)
        {
            // x is pitch, y is yaw
            Vector3 euler = cameraObject.eulerAngles;

            euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
            euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
            euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
            euler.y = Mathf.Clamp(euler.y, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);

            Debug.Print(euler.x);
            cameraObject.eulerAngles = euler;  
#if false
            Vector3 front = cameraObject.front;
            front.x = Mathf.Cos(euler.y) * Mathf.Cos(euler.x);
            front.y = Mathf.Sin(euler.x);
            front.z = Mathf.Sin(euler.y) * Mathf.Cos(euler.x);
            cameraObject.front = front;
#endif
        }
    }

}