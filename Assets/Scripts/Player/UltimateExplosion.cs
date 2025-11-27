// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class UltimateExplosion : Script
{
    // ======================================
    // Serialised fields.
    // ======================================
    public MeshRenderer_ material;
    public float fadeInDuration = 1f;
    public float explosionDuration = 1f;
    public float fadeOutDuration = 2f;

    public float initialScale = 1f;
    public float explosionInitialScale = 20f;
    public float explosionFinalScale = 25f;
    public float finalScale = 30f;

    public float angularVelocity = 100f;

    // ======================================
    // Runtime variables.
    // ======================================
    private float timeElapsed = 0f;
    private Transform_ transform;

    private Vector3 initialScaleVector;
    private Vector3 explosionInitialScaleVector;
    private Vector3 explosionFinalScaleVector;
    private Vector3 finalScaleVector;

    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = gameObject.transform;
    }

    // This function is invoked every update.
    protected override void update()
    {
        // Handle fade in lerp..
        if (timeElapsed < fadeInDuration)
        {

        }
        // Handle explosion stay in...
        else if (timeElapsed < fadeInDuration + explosionDuration) 
        { 
            float relativeTimeElapsed = timeElapsed - fadeInDuration;

            float interval = relativeTimeElapsed / explosionDuration;
        }
        // Handle fade out lerp..
        else if(timeElapsed < fadeInDuration + explosionDuration + fadeOutDuration)
        {
            float relativeTimeElapsed = timeElapsed - fadeInDuration - explosionDuration;
            
            float interval = relativeTimeElapsed / fadeOutDuration;
        }
        else
        {
            Destroy(gameObject);
            return;
        }

        transform.rotate(Vector3.Up(), angularVelocity);
        timeElapsed += Time.V_DeltaTime();
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}