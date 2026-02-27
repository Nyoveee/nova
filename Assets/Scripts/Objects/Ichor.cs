// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using Windows.UI.Composition;

class Ichor : Script
{
    private delegate void CurrentState();

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

    [SerializableField]
    private Vector3 idlePositionVariance = new Vector3(1f, 1f, 1f);

    [SerializableField]
    private float pullingForce = 20f;

    [SerializableField]
    private ColorAlpha color;

    /***********************************************************
        Local Variables
    ***********************************************************/

    private bool waitNextFrame = true;
    private Rigidbody_ rigidbody;
    private ParticleEmitter_ particleEmitter;
    
    private GameObject thrownGunGameObject;
    private GameObject heldGunGameObject;
    
    private Vector3 startSize;
    private Vector3 startPosition;

    private float elapsedTime = 0;
    private float pullElapsedTime = 0;

    private Vector3 idlePosition;

    // State handling..
    enum State
    {
        Spawning,
        Idle,
        GettingPulled
    }

    State currentState = State.Spawning;
    Dictionary<State, CurrentState> updateState = new Dictionary<State, CurrentState>();

    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        startSize = gameObject.transform.scale;
        Vector3 direction = new Vector3(Random.Range(-1f, 1f), Random.Range(-1f, 1f), Random.Range(-1f, 1f));
        direction.Normalize();
        Vector3 force =  direction * Random.Range(minForce, maxForce);
        rigidbody.AddForce(force);
        rigidbody.SetGravityFactor(Random.Range(minGravityFactor, maxGravityFactor));

        // Lifespan..
        Invoke(() =>
        {
            if(gameObject!= null)
                Destroy(gameObject);
        }, ichorLifeTime);

        // State machine function delegate setup
        updateState.Add(State.Spawning, UpdateSpawning);
        updateState.Add(State.Idle, UpdateIdle);
        updateState.Add(State.GettingPulled, UpdateAbsorbing);

        particleEmitter = getComponent<ParticleEmitter_>();

        if(particleEmitter != null)
        {
            ColorAlpha colorAlpha = new ColorAlpha(color.r / 255f, color.g / 255f, color.b / 255f, color.a / 255f);
            particleEmitter.setParticleColor(colorAlpha);
        }
    }

    protected void UpdateSpawning()
    {
        const float epilson = 0.1f;

        // During spawning, we are responsible for
        // 1. Expanding the game object's scale
        // 2. After a set amount of time, set linear damping to slow it down..
        elapsedTime += Time.V_DeltaTime();

        if (elapsedTime < expandTime)
        {
            gameObject.transform.scale = Vector3.Lerp(startSize, startSize * expandedSizeMultiplier, elapsedTime / expandTime);
        }

        if (elapsedTime > rapidSlowTime)
        {
            rigidbody.SetLinearDamping(damping);
        }

        // Ichor has stopped, transitioning to idle state.
        // (can't compare to zero, because of the way we dampen velocity, so let's compare to some threshold..)
        Vector3 velocity = rigidbody.GetVelocity();

        if (velocity.x < epilson && velocity.y < epilson && velocity.z < epilson)
        {
            currentState = State.Idle;

            rigidbody.SetVelocity(Vector3.Zero());
            rigidbody.SetLinearDamping(damping);

            idlePosition = gameObject.transform.position;
        }
    }

    protected void UpdateIdle()
    {
        // We want the ichor to wiggle around it's idle position..

        // We calculate the next random position..
        Vector3 randomPosition = idlePosition + Random.Range(-idlePositionVariance, idlePositionVariance);
        Vector3 forceVector = randomPosition - gameObject.transform.position;
        rigidbody.AddVelocity(forceVector);
    }

    protected void UpdateAbsorbing()
    {
        // If the thrown gun game object is valid, take it's position.. else, take the held gun's position.
        Vector3 endPosition = thrownGunGameObject != null ? thrownGunGameObject.transform.position : heldGunGameObject.transform.position;

#if true
        pullElapsedTime += Time.V_DeltaTime();

        float t = pullElapsedTime / pullTime;
        
        // Difference in lerping power to "create" an parabola instead of a straight line..
        gameObject.transform.position = new Vector3(
            Mathf.Interpolate(startPosition.x, endPosition.x, t, 2.5f),
            Mathf.Interpolate(startPosition.y, endPosition.y, t, 0.2f),
            Mathf.Interpolate(startPosition.z, endPosition.z, t, 1f)  
        );

        if (t > 1)
        {
            if (gameObject != null)
                Destroy(gameObject);
        }
#else
        // Instead of linear interpolation, let's try force based movement..


        // We don't normalize the direction, because we want the force to scale proportionally based on distancer as well..
        Vector3 pullingDirection = endPosition - gameObject.transform.position;
        pullingDirection *= pullingForce;

        rigidbody.AddForce(pullingDirection);
#endif
    }

    protected override void update()
    {
        // runs the update function of the current state.
        updateState[currentState]();
    }

    public void PullTowardsGun(GameObject p_thrownGunGameObject, GameObject p_heldGunGameObject)
    {
        startPosition = gameObject.transform.position;
        currentState = State.GettingPulled;
        thrownGunGameObject = p_thrownGunGameObject;
        heldGunGameObject = p_heldGunGameObject;

        if (particleEmitter != null)
        {
            particleEmitter.enable = true;
        }

        particleEmitter.setParticleColor(new ColorAlpha(1f, 1f, 0.3f, 1f));
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