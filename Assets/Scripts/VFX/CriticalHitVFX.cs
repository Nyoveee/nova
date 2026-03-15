// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class CriticalHitVFX : Script
{

    [SerializableField]
    private float totalDuration = 0.5f;
    [SerializableField]
    private float holdDuration = 0.1f;
    [SerializableField]
    private ParticleEmitter_ initialBurst = null;
    [SerializableField]
    private ParticleEmitter_ initialPoint = null;
    [SerializableField]
    private ParticleEmitter_ holdPoint = null;
    [SerializableField]
    private ParticleEmitter_ holdBurst = null;
    [SerializableField]
    private ParticleEmitter_ sparkBlue = null;
    [SerializableField]
    private ParticleEmitter_ sparkOrange = null;


    private float elaspedTime = 0.0f;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        //initialBurst.emit();
        //initialPoint.emit();
        //sparkBlue.emit();
        //sparkOrange.emit();

    }

    // This function is invoked every update.
    protected override void update()
    {
        if (elaspedTime == 0)
        {
            initialBurst.emit();
            initialPoint.emit();
            sparkBlue.emit();
            sparkOrange.emit();

        }



        elaspedTime += Time.V_DeltaTime();
        if (elaspedTime >= holdDuration)
        {
            //Time.timeScale = 0.0f;
            holdPoint.emit();
            holdBurst.emit();

        }

        if (elaspedTime >= totalDuration)
        {
            Destroy(this.gameObject);
        
        }
    
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}