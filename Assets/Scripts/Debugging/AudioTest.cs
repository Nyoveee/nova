// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class AudioTest : Script
{
    // This function is invoked once before init when gameobject is active.

    [SerializableField]
    public Audio audio;


    [SerializableField]
    public float timeStep = 1.0f;


    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        //Debug.Log("AudioTest init");    

        getComponent<AudioComponent_>().PlayBGM(audio);

    }

    // This function is invoked every update.
    protected override void update()
    {
    
    
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}