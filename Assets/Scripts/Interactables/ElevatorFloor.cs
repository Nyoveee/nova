// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Runtime.CompilerServices;

class ElevatorFloor : Script
{
    [SerializableField]
    private Switch elevatorSwitch;

    [SerializableField]
    private float duration = 3f;

    [SerializableField]
    private float lerpPower = 0.7f;

    [SerializableField]
    private float distance = 10f;

    [SerializableField]
    private float delay = 0.7f;

    private float timeElapsed = 0f;
    private bool isMoving = false;
    private bool toAnimate = false;

    private Vector3 initialPosition;
    private Vector3 finalPosition;

    private GameObject playerBody;
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        initialPosition = gameObject.transform.localPosition;
        finalPosition = gameObject.transform.localPosition + new Vector3(0, distance, 0);

        playerBody = GameObject.FindWithTag("Player");
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(toAnimate && timeElapsed < duration)
        {
            timeElapsed += Time.V_DeltaTime();
            timeElapsed = Mathf.Min(timeElapsed, duration);
            
            float interval = timeElapsed / duration;


            Vector3 startPos = gameObject.transform.localPosition;
            gameObject.transform.localPosition = Vector3.Lerp(initialPosition, finalPosition, Mathf.Pow(Mathf.SmoothLerp(0f, 1f, interval), lerpPower));
            
            Vector3 difference = gameObject.transform.localPosition - startPos;

            Debug.Log(difference);

            if (playerBody != null)
            {
                playerBody.transform.position += difference * 3;
            }
        }
        if (!isMoving && elevatorSwitch != null && elevatorSwitch.isSwitchActivated()) {
            isMoving = true;

            Invoke(() =>
            {
                toAnimate = true;
            }, delay);
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}