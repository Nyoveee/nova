// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerController : Script
{
    public float cameraSensitivity = 0.1f;
    
    [SerializableField]
    private Transform_? cameraObject = null;
    // This function is first invoked when game starts.
    protected override void init()
    {
        CameraAPI.LockMouse();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        CameraMovement();   
    }
    private void CameraMovement()
    {

        if (cameraObject != null)
        {
            // x is pitch, y is yaw
            Vector3 euler = cameraObject.eulerAngles;

            euler.y -= cameraSensitivity * Input.GetMouseAxis(MouseAxis.Horizontal);
            euler.x -= cameraSensitivity * Input.GetMouseAxis(MouseAxis.Vertical);
            euler.x = Mathf.Clamp(euler.x, -89.0f * Mathf.Deg2Rad, 89.0f * Mathf.Deg2Rad);

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