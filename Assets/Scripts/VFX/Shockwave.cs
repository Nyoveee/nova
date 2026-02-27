// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Shockwave : Script
{
    [SerializableField] private float damage = 40f;
    [SerializableField] private float knockbackMultiplier = 10f;

    [SerializableField] private float maxRadius = 100f;
    [SerializableField] private float duration = 2.2f;
    [SerializableField] private float fadeOutDuration = 1f;
    [SerializableField] private float emissiveMultiplier = 100f;
    [SerializableField] private float motionLerpPower = 1f;

    private float timeElapsed = 0f;
    private MeshRenderer_ meshRenderer;
    private float initialParticleRate = 1f;
    private float initialParticleRadius = 1f;

    private bool hasDamagedPlayer = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    { }

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        CameraAPI.shakeCamera(0.6f, 1f);
        meshRenderer = getComponent<MeshRenderer_>();

        foreach (GameObject child in gameObject.GetChildren())
        {
            ParticleEmitter_ emitter = child.getComponent<ParticleEmitter_>();

            if (emitter != null)
            {
                initialParticleRate = emitter.particleRate;
                initialParticleRadius = emitter.getRadius();
                break;
            }
        }

        getComponent<ParticleEmitter_>()?.emit(500);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (timeElapsed > duration)
        {
            Destroy(gameObject);
            return;
        }

        float interval = timeElapsed / duration;
        float lerpedSize = Mathf.Interpolate(0, maxRadius, interval, motionLerpPower);
        float heightSize = Mathf.Interpolate(0f, maxRadius / 2f, interval, motionLerpPower);
        gameObject.transform.scale = new Vector3(lerpedSize, heightSize, lerpedSize);
        timeElapsed += Time.V_DeltaTime();

        // Expand the emission size of the particle emitter..
        foreach (GameObject child in gameObject.GetChildren())
        {
            ParticleEmitter_ emitter = child.getComponent<ParticleEmitter_>();

            if (emitter != null)
            {
                emitter.setRadius(initialParticleRadius * lerpedSize);
                emitter.particleRate = initialParticleRate * lerpedSize;
            }
        }

        if (timeElapsed > duration - fadeOutDuration)
        {
            interval = Mathf.Pow((timeElapsed - (duration - fadeOutDuration)) / (fadeOutDuration), 0.4f);

            meshRenderer.setMaterialFloat(0, "emissivemultiplier", emissiveMultiplier * (1 - interval));
            meshRenderer.setMaterialFloat(0, "transparency", (1 - interval));
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    { }

    // This function is invoked when destroyed.
    protected override void exit()
    { }

    protected override void onCollisionEnter(GameObject other)
    {
        if(hasDamagedPlayer)
        {
            return;
        }

        if (other.tag != "Player")
        {
            return;
        }

        hasDamagedPlayer = true;
        other.getScript<PlayerController_V2>()?.TakeDamage(damage);

        Vector3 forceDirection = other.transform.position - gameObject.transform.position;
        forceDirection.Normalize();
        forceDirection *= knockbackMultiplier;
        other.getComponent<Rigidbody_>().AddImpulse(forceDirection);
    }
}