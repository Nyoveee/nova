// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class StompAttackVFX : Script
{
    [SerializableField] private float movingDuration = 2f;
    [SerializableField] private float speed = 50f;
    [SerializableField] private float sizeIncrease = 2f;
    [SerializableField] private float fadeOut = 0.3f;
    [SerializableField] private float damage = 20f;
    [SerializableField] private List<Audio> waveAudio;

    MeshRenderer_ meshRenderer;

    bool isMoving = false;
    float timeElapsed = 0f;

    Vector3 initialScale;
    Vector3 finalScale;

    bool hasDamagedPlayer = false;

    // This function is invoked once when gameobject is active.
    protected override void awake()
    {
        initialScale = gameObject.transform.scale;
        finalScale = initialScale * sizeIncrease;

        meshRenderer = getComponent<MeshRenderer_>();
    }

    protected override void init()
    {
        getComponent<AudioComponent_>().PlayRandomSound(waveAudio);
        move();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!isMoving)
        {
            return;
        }

        // Scaling happens throughout..
        float interval = timeElapsed / movingDuration;
        gameObject.transform.scale = Vector3.Lerp(initialScale, finalScale, interval);
        
        // Constant velocity throughout before fade out..
        if(timeElapsed < movingDuration)
        {
            gameObject.transform.position += gameObject.transform.front * speed * Time.V_DeltaTime();
        }
        // Fading out..
        else
        {
            interval = 1 - ((timeElapsed - movingDuration) / fadeOut);
            meshRenderer.setMaterialFloat(0, "alphaMultiplier", interval);
            gameObject.transform.position += gameObject.transform.front * speed * interval * Time.V_DeltaTime();
        }

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > (movingDuration + fadeOut))
        {
            isMoving = false;
            Destroy(gameObject);
        }
    }

    public void move()
    {
        timeElapsed = 0f;
        isMoving = true;
    }

    protected override void onCollisionEnter(GameObject other)
    {
        if (hasDamagedPlayer)
        {
            return;
        }

        if (other.tag != "Player")
        {
            return;
        }

        hasDamagedPlayer = true;
        other.getScript<PlayerController_V2>()?.TakeDamage(damage);
    }
}