// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class HeadShake : Script
{

    [SerializableField]
    public float rotationAmount = 45f;

    [SerializableField]
    public float speed = 2f;

    private Quaternion startRotation;

    private float baseY = 0f;

    private float timeElasped = 0f;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        startRotation = gameObject.transform.rotation;
        baseY = gameObject.transform.eulerAngles.y;
    }

    // This function is invoked every update.
    protected override void update()
    {

        //timeElasped += Time.V_DeltaTime();

        //float angle = Mathf.Sin(timeElasped * speed) * rotationAmount;

        //gameObject.transform.eulerAngles = new Vector3(0, baseY + angle,0);

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}