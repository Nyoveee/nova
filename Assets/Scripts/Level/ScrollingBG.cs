// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Media.Transcoding;
using Windows.Services.Maps.LocalSearch;

class ScrollingBG : Script
{

    [SerializableField]
    private float scrollSpeed = 50f;

    [SerializableField]
    private float recycleZ = -20f;

    [SerializableField]
    private float resetZ = 40f;

    [SerializableField]
    private GameObject lever; 

    private bool isScrolling = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {
        if (lever.getScript<Switch>().isSwitchActivated() == true)
        {
            if (isScrolling == false)
            {
                isScrolling = true;
            }
        }
        if(isScrolling == true)
        {
            Vector3 pos = gameObject.transform.position;
            pos.z -= scrollSpeed * Time.V_DeltaTime();
            gameObject.transform.position = pos;

            // Simple teleport if past recycle point
            if (pos.z < recycleZ)
            {
                pos.z = resetZ;
                gameObject.transform.position = pos;
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void StopScrolling()
    {
        isScrolling = false;
        scrollSpeed = 0;
    }
}