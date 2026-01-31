// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class UltimateExplosion : Script
{
    // ======================================
    // Serialised fields.
    // ======================================
    public float fadeInDuration = 1f;
    public float explosionDuration = 1f;
    public float collapseDuration = 0.5f;
    public float fadeOutDuration = 2f;

    public float dissolveOffsetDuration = 0.2f;

    public float initialScale = 1f;
    public float explosionInitialScale = 20f;
    public float explosionFinalScale = 25f;
    public float finalScale = 30f;

    public float lightIntensity = 100f;

    public float angularVelocity = 100f;

    // ======================================
    // Runtime variables.
    // ======================================
    private float timeElapsed = 0f;
    private Transform_ transform;
    private MeshRenderer_ material;
    private Light_ light;
    private List<GameObject> enemyObject = new List<GameObject>();
    private Vector3 initialScaleVector;
    private Vector3 explosionInitialScaleVector;
    private Vector3 explosionFinalScaleVector;
    private Vector3 finalScaleVector;

    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = gameObject.transform;
        transform.scale = Vector3.One();
        material = getComponent<MeshRenderer_>();
        light = getComponent<Light_>();

        initialScaleVector          = new Vector3(initialScale, initialScale, initialScale);
        explosionInitialScaleVector = new Vector3(explosionInitialScale, explosionInitialScale, explosionInitialScale);
        explosionFinalScaleVector   = new Vector3(explosionFinalScale, explosionFinalScale, explosionFinalScale);
        finalScaleVector            = new Vector3(finalScale, finalScale, finalScale);

        // AudioAPI.PlaySound(gameObject, "sniper_specialImpact_01");
    }

    // This function is invoked every update.
    protected override void update()
    {
        // Handle fade in lerp..
        if (timeElapsed < fadeInDuration)
        {
            float interval = timeElapsed / fadeInDuration;
            RendererAPI.exposure = Mathf.Interpolate(0.9f, 0.2f, interval, 1f);
            transform.scale = Vector3.Lerp(initialScaleVector, explosionInitialScaleVector, Mathf.Pow(interval, 0.2f));
            light.intensity = Mathf.Interpolate(0f, lightIntensity, interval, 1f);
        }
        // Handle explosion stay in...
        else if (timeElapsed < fadeInDuration + explosionDuration) 
        {
            // RendererAPI.toneMapping = true;
            float relativeTimeElapsed = timeElapsed - fadeInDuration;
            float interval = relativeTimeElapsed / explosionDuration;
            transform.scale = Vector3.Lerp(explosionInitialScaleVector, explosionFinalScaleVector, interval);
        }
        // Handle fade out lerp..
        else if (timeElapsed < fadeInDuration + explosionDuration + collapseDuration)
        {
            float relativeTimeElapsed = timeElapsed - fadeInDuration - explosionDuration;

            float interval = Mathf.Pow(relativeTimeElapsed / collapseDuration, 3f);

            transform.scale = Vector3.Lerp(explosionFinalScaleVector, Vector3.Zero(), interval);
        }
        // Handle fade out lerp..
        else if(timeElapsed < fadeInDuration + explosionDuration + collapseDuration + fadeOutDuration)
        {
            float relativeTimeElapsed = timeElapsed - fadeInDuration - explosionDuration - collapseDuration;

            float interval = Mathf.Pow(relativeTimeElapsed / fadeOutDuration, 0.3f);

            transform.scale = Vector3.Lerp(Vector3.Zero(), finalScaleVector, interval);

            material.setMaterialFloat(0, "dissolveThreshold", 1f - interval);
            light.intensity = Mathf.Interpolate(lightIntensity, 0f, interval, 1f);
            RendererAPI.exposure = Mathf.Interpolate(0.2f, 0.9f, interval, 1f);
        }
        else
        {
            // RendererAPI.toneMapping = false;
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
    {
        
    }

    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Enemy_ArmouredSpot" || other.tag == "Enemy_WeakSpot")
        {
            EnemyCollider enemyCollider = other.getScript<EnemyCollider>();

            if (enemyCollider != null)
            {
                GameObject enemy = other.GetParent();

                if (enemy != null && !enemyObject.Contains(enemy))
                {
                    enemyObject.Add(enemy);

                    enemyCollider.OnColliderShot(200f, Enemy.EnemydamageType.Ultimate, other.tag); //ensure each enemy is hit only once
                }
            }
        }
    }
}