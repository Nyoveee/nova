class TestScript : Script
{
    [SerializableField]
    private Transform_? transform = null;
    [SerializableField]
    private Vector3 spawnPos = new Vector3(0, 0, 0);
    [SerializableField]
    private float moveSpeed = 0f;

    protected override void init()
    {
        transform = getComponent<Transform_>();
        if (transform != null)
        {
            transform.position = spawnPos;
        }
    }
    protected override void update()
    {
        if(transform!= null)
        {
            Vector3 position = transform.position;
            position.z += moveSpeed * Time.V_FixedDeltaTime();
            transform.position = position;
        }
       
    }
}
