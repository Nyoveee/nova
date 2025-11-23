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
    private ParticleEmitter_? emitter;
    private bool waitNextFrame = true;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private Rigidbody_ rigidbody;
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        emitter = getComponent<ParticleEmitter_>();
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
            Vector3 force = new Vector3(Random.Range(-500f, 500f), Random.Range(1f, 100f), Random.Range(-500f, 500f));
            rigidbody.AddForce(force);
            waitNextFrame = false;
        }
    }
    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Floor")
            rigidbody.enable = false;
    }
}