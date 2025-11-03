// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class BulletScript : Script
{
    public required float lifeTime = 10f;

    private float timeElapsed = 0;

    // This function is first invoked when game starts. s
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {
        // 
        if(timeElapsed > lifeTime)
        {
            ObjectAPI.Destroy(gameObject);
        }
        else
        {
            timeElapsed += Time.V_FixedDeltaTime();
        }
    }

    protected override void onCollisionEnter(GameObject other)
    {
        Debug.Log("Collide!");
        
        if (other.tag == "Enemy" || other.tag == "Wall")
        {
            Debug.Log("Collide!");
            ObjectAPI.Destroy(gameObject);
        }
    }
}