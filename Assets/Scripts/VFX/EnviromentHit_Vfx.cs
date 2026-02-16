// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnviromentHit_Vfx : Script
{

    // ==================================
    // Parameters
    // ==================================
    [SerializableField]
    public float totalDuration;
    [SerializableField]
    public Light_ lightBurst;
    [SerializableField]
    public ParticleEmitter_ bigSpark;
    [SerializableField]
    public ParticleEmitter_ smallSpark;

    // ==================================
    // Private Variables
    // ==================================
    private float timeElapsed = 0f;
    private float intialIntensity   = 0f;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        intialIntensity = lightBurst.intensity;
        bigSpark.emit();
        smallSpark.emit();
    
    }




    // This function is invoked every update.
    protected override void update()
    {
        timeElapsed +=  Time.V_DeltaTime();

        lightBurst.intensity = float.Lerp(intialIntensity, 0f, timeElapsed / totalDuration);


        if(timeElapsed > totalDuration)
        {
            if (gameObject != null)
                Destroy(gameObject);
        }
    }




}