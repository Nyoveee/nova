// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PushField : Script
{

    [SerializableField]
    private float duration = 1.0f;

    [SerializableField]
    private float force = 100f;

    [SerializableField]
    private float distance = 20f;

    private GameObject player = null;
    private float      timeElasped = 0.0f;
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        var objects = GameObject.FindGameObjectsWithTag("Player");

        player = (GameObject)objects[0];
    
    }

    // This function is invoked every update.
    protected override void update()
    {
        timeElasped += Time.V_DeltaTime();

        if (timeElasped >= duration)
        {
            Destroy(this.gameObject);
            return;
        }

        if (player == null)
        {
            return;
        }
    


        Vector3 toPlayer = player.transform.position - gameObject.transform.position;

        toPlayer.y = 0;

        if (toPlayer.Length() < distance)
        {
            toPlayer.Normalize();
            player.getComponent<Rigidbody_>().AddImpulse(toPlayer * force);
        
        }

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}