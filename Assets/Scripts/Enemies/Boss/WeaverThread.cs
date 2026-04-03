// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class WeaverThread : Script
{
    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float startUpTime = 0.2f; //we will play an animation of the laser spawning before its travels
    [SerializableField]
    private float threadDistance = 1f; //scale the thread by some distance


    [SerializableField]
    public GameObject startPointObject;

    [SerializableField]
    public GameObject weaverThread;


    /***********************************************************
    Runtime variables..
    ***********************************************************/
    private float timeElapsed = 0f;
    private float baseScale = 1f;
    private bool  hasClampedDistance = false;


    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        baseScale = weaverThread.transform.localScale.z;
    
    }

    // This function is invoked every update.
    protected override void update()
    {
        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed <= startUpTime)
        {
            float scaledDistance = Mathf.SmoothLerp(baseScale, threadDistance, timeElapsed/startUpTime);

            weaverThread.transform.scale = new Vector3 (weaverThread.transform.scale.x, weaverThread.transform.scale.y, scaledDistance);
        }
        else
        {
            if (!hasClampedDistance)
            {
                weaverThread.transform.scale = new Vector3(weaverThread.transform.scale.x, weaverThread.transform.scale.y, threadDistance);
                hasClampedDistance = true;
            }
        
        }
    
    
    


    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}