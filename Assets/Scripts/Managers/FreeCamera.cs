// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class FreeCamera : Script
{
    public float cameraSensitivity = 1.0f;
    public float maxSpeed = 5f;
    public float deceleration = 0.9f;

    bool isPanning = false;
    Vector3 velocity = Vector3.Zero();

    bool isMovingForward = false;
    bool isMovingBack = false;
    bool isMovingLeft = false;
    bool isMovingRight = false;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        MapKey(Key.MouseRight, onRightClickPressed, onRightClickReleased);
        MouseMoveCallback(CameraMovement);
        CameraAPI.LockMouse();

        MapKey(Key.W, 
        ()=>
        {
            isMovingForward = true;
        },
        
        () =>
        {
            isMovingForward = false;
        });

        MapKey(Key.A,
        () =>
        {
            isMovingLeft = true;
        },

        () =>
        {
            isMovingLeft = false;
        });

        MapKey(Key.S, 
        ()=>
        {
            isMovingBack = true;
        },
        
        () =>
        {
            isMovingBack = false;
        });

        MapKey(Key.D, 
        ()=>
        {
            isMovingRight = true;
        },
        
        () =>
        {
            isMovingRight = false;
        });

        ScrollCallback((float delta) =>
        {
            deceleration += delta * 0.005f;
        });
    }

    protected override void update()
    {
        bool isMoving = isMovingForward || isMovingBack || isMovingLeft || isMovingRight;

        //float accelerationFactor = ((maxSpeed - velocity.Length()) / (maxSpeed));
        float accelerationFactor =  1f;
        //Debug.Log(accelerationFactor);

        if (isMovingForward)
        {
            velocity += gameObject.transform.front * accelerationFactor;
        }

        if (isMovingBack)
        {
            velocity -= gameObject.transform.front * accelerationFactor;
        }

        if (isMovingLeft)
        {
            velocity -= gameObject.transform.right * accelerationFactor;
        }

        if (isMovingRight)
        {
            velocity += gameObject.transform.right * accelerationFactor;
        }

        if(!isMoving)
        {
        }

        velocity *= deceleration;
        gameObject.transform.position += velocity * Time.V_DeltaTime();
    } 

    void onRightClickPressed()
    {
        isPanning = true;
    }

    void onRightClickReleased()
    {
        isPanning = false;
    }

    private void CameraMovement(float deltaMouseX, float deltaMouseY)
    {
        //if (!isPanning)
        //{
        //    return;
        //}

        //Vector3 euler = gameObject.transform.localEulerAngles;

        //euler.x -= cameraSensitivity * deltaMouseY * Time.V_DeltaTime();
        //euler.y -= cameraSensitivity * deltaMouseX * Time.V_DeltaTime();
        //euler.x = Mathf.Clamp(euler.x, -80.0f * Mathf.Deg2Rad, 80.0f * Mathf.Deg2Rad);
        //gameObject.transform.localEulerAngles = euler;

        Vector3 lookPos = gameObject.transform.front + gameObject.transform.right * cameraSensitivity * deltaMouseX;
        lookPos += gameObject.transform.up * cameraSensitivity * deltaMouseY;
        lookPos.Normalize();

        gameObject.transform.setFront(lookPos);
    }
}