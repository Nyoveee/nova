// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using Windows.Devices.Display.Core;
using Windows.UI.Text;

class PlayerController : Script
{
    // ==================================
    // Parameters to be tweaked
    // ==================================
    public float cameraSensitivity = 0.03f;
    public float moveSpeed = 5f;
    public float jumpStrength = 100f;

    [SerializableField]
    private Transform_? cameraObject = null;

    // ==================================
    // Internal
    // ==================================
    private Transform_? transform;
    private Rigidbody_? rigidbody;

    private bool isMovingForward = false;
    private bool isMovingBackward = false;
    private bool isMovingLeft = false;
    private bool isMovingRight = false;
    private bool isGrounded = false;

    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();

        CameraAPI.LockMouse();

        // Controls..
        Input.MouseMoveCallback(CameraMovement);

        Input.MapKey(Key.W, beginWalkingForward, endWalkingForward);
        Input.MapKey(Key.A, beginWalkingLeft, endWalkingLeft);
        Input.MapKey(Key.S, beginWalkingBackward, endWalkingBackward);
        Input.MapKey(Key.D, beginWalkingRight, endWalkingRight);

        Input.MapKey(Key.Space, jump);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        // Check if its grounded..
        var result = PhysicsAPI.Raycast(transform.position, Vector3.Down, 1f, gameObject);

        if (result != null) { 
            isGrounded = true;
        }
        else
        {
            isGrounded = false;
        }

        handleMovement();
    }

    private void handleMovement()
    {
        // ==============================
        // Handles WASD movement..
        // ==============================
        Vector3 orientedFront = new Vector3(cameraObject.front.x, 0, cameraObject.front.z);
        Vector3 orientedRight = new Vector3(cameraObject.right.x, 0, cameraObject.right.z);

        Vector3 directionVector = Vector3.One;

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

        // This frame, user has at least pressed one movement key..
        if(isMoving)
        {
            directionVector.Normalize();
            Vector3 movingVector = directionVector * moveSpeed;
            Vector3 newVelocity = new Vector3(movingVector.x, rigidbody.GetVelocity().y, movingVector.z);

            rigidbody.SetVelocity(newVelocity);
        }

        // Stop user from moving if grounded..
        else if (isGrounded)
        {
            rigidbody.SetVelocity(new Vector3(0f, 0f, 0f));
        }
    }

    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        // We rotate our parent in the y axis..
        // transform.rotate(Vector3.Up, deltaMouseX * cameraSensitivity);
        //transform.rotate(Vector3.Front, deltaMouseY * cameraSensitivity);

        // x is pitch, y is yaw
        Vector3 euler = cameraObject.localEulerAngles;

        euler.x -= cameraSensitivity * deltaMouseY;
        euler.y -= cameraSensitivity * deltaMouseX;
        euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);

        // euler.y = Mathf.Clamp(euler.y, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);

        cameraObject.localEulerAngles = euler;  
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

    private void jump()
    {
        rigidbody.AddForce(Vector3.Up * jumpStrength);
    }
}