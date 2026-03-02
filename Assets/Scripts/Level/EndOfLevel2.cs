// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EndOfLevel2 : Script
{
    [SerializableField]
    private float targetZ = -170f;
    [SerializableField]
    private float scrollSpeed = 50f;
    [SerializableField]
    private GameObject[] stopScroll;
    [SerializableField]
    private bool start = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        stopScroll = GameObject.FindGameObjectsWithTag("Scroll");
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (start == true)
        {
            Vector3 pos = gameObject.transform.position;
            if (pos.z > targetZ)
            {
                pos.z -= scrollSpeed * Time.V_DeltaTime();
                gameObject.transform.position = pos;
            }
            else
            {
                foreach (var scroll in stopScroll)
                {
                    scroll.getScript<ScrollingBG>().StopScrolling();
                }
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void StartScroll()
    {
        start = true;
    }
}