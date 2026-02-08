// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using Windows.UI.Composition;

class Ichor : Script
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float ichorLifeTime = 30f;
    [SerializableField]
    private float expandedSizeMultiplier = 1.5f;
    [SerializableField]
    private float expandTime = 1f;
    [SerializableField]
    private float pullTime = 0.2f;
    [SerializableField]
    private float minForce = 10f;
    [SerializableField]
    private float maxForce = 10f;
    [SerializableField]
    private float rapidSlowTime = 1f;
    [SerializableField]
    private float minGravityFactor = 1f;
    [SerializableField]
    private float maxGravityFactor = 1f;
    [SerializableField]
    private float damping = 1f;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private bool waitNextFrame = true;
    private Rigidbody_ rigidbody;
    private Vector3 startSize;
    private bool gunPull = false;
    private Vector3 startDistance;
    private Vector3 endDistance;

    private float elapsedTime = 0;
    private float pullElapsedTime = 0;
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        startSize = gameObject.transform.scale;
        Vector3 direction = new Vector3(Random.Range(-1f, 1f), Random.Range(-1f, 1f), Random.Range(-1f, 1f));
        direction.Normalize();
        Vector3 force =  direction * Random.Range(minForce, maxForce);
        rigidbody.AddForce(force);
        rigidbody.SetGravityFactor(Random.Range(minGravityFactor, maxGravityFactor));
        Invoke(() =>
        {
            Destroy(gameObject);
        }, ichorLifeTime);
    }
    protected override void update()
    {
        // Make sure collision is only affectted by the newly set postiion
        //expandTime = Mathf.Max(0, expandTime - Time.V_DeltaTime());
        //gameObject.transform.scale = Vector3.Lerp(startSize, startSize * expandedSizeMultiplier, expandTime);
        elapsedTime += Time.V_DeltaTime();

        if(elapsedTime < expandTime)
        {
            gameObject.transform.scale = Vector3.Lerp(startSize, startSize * expandedSizeMultiplier, elapsedTime / expandTime);
        }

        if (elapsedTime > rapidSlowTime)
        { 
            rigidbody.SetLinearDamping(damping);

        }
       

        if (gunPull == true)
        {
            pullElapsedTime += Time.V_DeltaTime();

            float t = pullElapsedTime / pullTime;
            gameObject.transform.position = Vector3.Lerp(startDistance, endDistance,t );

            if (t > 1)
            { 
                Destroy(gameObject);
            
            }
        
        }


    }


    public void PullTowardsGun(Vector3 position)
    {
        gunPull = true;
        startDistance = gameObject.transform.position;
        endDistance = position;
    }

    protected override void onCollisionEnter(GameObject other)
    {
        //if (other.tag == "Floor" 
        //    || other.tag == "Props" 
        //    || other.getComponent<Rigidbody_>().GetLayerName() == "Floor" 
        //    || other.getComponent<Rigidbody_>().GetLayerName() == "Props"
        //    || other.getComponent<Rigidbody_>().GetLayerName() == "Wall")
        //   rigidbody.SetVelocity(Vector3.Zero());

    }
}