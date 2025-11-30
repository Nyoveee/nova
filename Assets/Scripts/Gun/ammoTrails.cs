// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ammoTrails: Script
{
    [SerializableField]
    public Vector3 endPosition;
    [SerializableField]
    public Vector3 startPosition;
    [SerializableField]
    public float killTime = 0.1f;
    [SerializableField]
    public float totalDuration = 0.1f;

    private bool awaitDeath = false;

    private float timeElapsed = 0;
    // This function is first invoked when game starts.
    protected override void init()
    {



    }

    // This function is invoked every update.
    protected override void update()
    {

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > totalDuration && awaitDeath == false)
        {
            Invoke(Kill, killTime);
            awaitDeath = true;
        }


        float t = timeElapsed / totalDuration;
        if (t <= 1)
        {

            gameObject.transform.position  = Vector3.Lerp(startPosition, endPosition, t);
        }



    }



    // This function is invoked every update.
    protected override void fixedUpdate()
    { }


    void Kill()
    {
        Destroy(gameObject);
    }


    // This function is invoked when destroyed.
    protected override void exit()
    { }
}