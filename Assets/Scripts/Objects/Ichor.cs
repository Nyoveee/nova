// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Ichor : Script
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float ichorLifeTime = 10f;
    [SerializableField]
    private float expandedSizeMultiplier = 1.5f;
    [SerializableField]
    private float expandTime = 1f;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private ParticleEmitter_? emitter;
    private bool waitNextFrame = true;
    private Rigidbody_ rigidbody;
    private Vector3 startSize;
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        emitter = getComponent<ParticleEmitter_>();
        startSize = gameObject.transform.scale;
        emitter.enable = false;
        rigidbody.enable = false;
        Invoke(() =>
        {
            Destroy(gameObject);
        }, ichorLifeTime);
    }
    protected override void update()
    {
        // Make sure emitter and collision is only affectted by the newly set postiion
        if (waitNextFrame){
            emitter.enable = true;
            rigidbody.enable = true;
            Vector3 force = new Vector3(Random.Range(-300f, 300f), Random.Range(1f, 700f), Random.Range(-300f, 300f));
            rigidbody.AddForce(force);
            waitNextFrame = false;
        }
        expandTime = Mathf.Max(0, expandTime - Time.V_FixedDeltaTime());
        gameObject.transform.scale = Vector3.Lerp(startSize, startSize * expandedSizeMultiplier, expandTime);
    }
    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Floor")
            rigidbody.enable = false;
    }
}