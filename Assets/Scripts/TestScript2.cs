class TestScript2: Script
{
    Transform_? transform;
    protected override void init()
    {
       transform = getComponent<Transform_>();
    }
    public void MoveTable()
    {
        if (transform != null)
        {
            Vector3 position = transform.position;
            position.z += Time.fixedDeltaTime;
            transform.position = position;
        }
    }
}