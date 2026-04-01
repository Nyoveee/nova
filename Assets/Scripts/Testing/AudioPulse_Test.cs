// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class AudioPulse_Test : Script
{

    [SerializableField]
    float timeStep = 1.0f;


    [SerializableField]
    public List<Audio> footsteps;

    float timeElapsed = 0f;


    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        getComponent<AudioComponent_>().PlayRandomSound(footsteps);


    }

    // This function is invoked every update.
    protected override void update()
    {
        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > timeStep)
        {
            getComponent<AudioComponent_>().PlayRandomSound(footsteps);

            timeElapsed = 0f;

        
        }

    
    
    
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}