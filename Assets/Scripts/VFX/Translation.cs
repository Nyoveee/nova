// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Translation : Script
{
    [SerializableField] private bool playOnStart = false;
    [SerializableField] private float expandDuration = 2f;
    [SerializableField] private float distance = 100f;
    [SerializableField] private float lerpPower = 1f;

    bool isExpanding = false;
    float timeElapsed = 0f;

    Vector3 initialPosition;
    Vector3 finalPosition;

    // This function is invoked once when gameobject is active.
    protected override void awake()
    {
        initialPosition = gameObject.transform.position;
        finalPosition = initialPosition + gameObject.transform.front * distance;
    }

    protected override void init()
    {
        if(playOnStart)
        {
            move();
        }
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!isExpanding)
        {
            return;
        }

        gameObject.transform.position = Vector3.Lerp(initialPosition, finalPosition, Mathf.Pow(timeElapsed / expandDuration, lerpPower));

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > expandDuration)
        {
            isExpanding = false;
        }
    }

    public void move()
    {
        timeElapsed = 0f;
        isExpanding = true;
    }

    public void stop()
    {
        isExpanding = false;
    }

}