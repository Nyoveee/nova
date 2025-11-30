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
    private float ichorLifeTime = 10f;
    [SerializableField]
    private float expandedSizeMultiplier = 1.5f;
    [SerializableField]
    private float expandTime = 1f;
    [SerializableField]
    private float pullTime = 0.2f;
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
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        startSize = gameObject.transform.scale;
        Vector3 force = new Vector3(Random.Range(-300f, 300f), Random.Range(1f, 700f), Random.Range(-300f, 300f));
        rigidbody.AddForce(force);
        Invoke(() =>
        {
            Destroy(gameObject);
        }, ichorLifeTime);
    }
    protected override void update()
    {
        // Make sure collision is only affectted by the newly set postiion
        expandTime = Mathf.Max(0, expandTime - Time.V_DeltaTime());
        gameObject.transform.scale = Vector3.Lerp(startSize, startSize * expandedSizeMultiplier, expandTime);


        if (gunPull == true)
        { 
            elapsedTime += Time.V_DeltaTime();

            float t = elapsedTime / pullTime;
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
        if (other.tag == "Floor")
           rigidbody.SetVelocity(Vector3.Zero());
    }
}