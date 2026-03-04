// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class StompAttackVFX : Script
{
    [SerializableField] private float movingDuration = 2f;
    [SerializableField] private float distance = 100f;
    [SerializableField] private float lerpPower = 1f;

    bool isMoving = false;
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
        move();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!isMoving)
        {
            return;
        }

        gameObject.transform.position = Vector3.Lerp(initialPosition, finalPosition, Mathf.Pow(timeElapsed / movingDuration, lerpPower));

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > movingDuration)
        {
            isMoving = false;
            Destroy(gameObject);
        }
    }

    public void move()
    {
        timeElapsed = 0f;
        isMoving = true;
    }
}