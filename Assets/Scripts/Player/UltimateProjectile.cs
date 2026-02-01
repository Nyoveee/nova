// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using static System.Net.Mime.MediaTypeNames;

class UltimateProjectile : Script
{
    // ==================================
    // Parameters
    // ==================================
    public Prefab ultimateExplosion;
    public float lifetime = 3f;
    [SerializableField]
    private Audio ultProjSFX;
    [SerializableField]
    private Audio ultExplodeSFX;
    // ===========================================
    // Components
    // ===========================================
    private AudioComponent_? audioComponent;

    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        //audioComponent.PlaySound(ultProjSFX);
        Invoke(() =>
        {
            if (gameObject != null)
            {
                //audioComponent.PlaySound(ultExplodeSFX);
                Instantiate(ultimateExplosion, gameObject.transform.position);
                Destroy(gameObject);
            }
        }, lifetime);
    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Wall" || other.tag == "Floor")
        {
            if (gameObject != null)
            {
                Instantiate(ultimateExplosion, gameObject.transform.position);
                Destroy(gameObject);
            }
        }
    }

}