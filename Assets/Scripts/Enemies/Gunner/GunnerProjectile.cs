// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GunnerProjectile : Script
{
    /***********************************************************
       Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float damage;
    [SerializableField]
    private float bulletSpeed;
    [SerializableField]
    private float lifetime;
    /***********************************************************
       Local Variables
    ***********************************************************/
    private Vector3 direction;
    public void SetDirection(Vector3 direction)
    {
        this.direction = direction;
    }
    protected override void init()
    {
        Invoke(() =>
        {
            if(gameObject!=null)
                Destroy(gameObject);
        }, lifetime);
    }
    protected override void update()
    {
        getComponent<Rigidbody_>().SetVelocity(direction * bulletSpeed);
    }
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Wall")
        {
            if (gameObject != null)
                Destroy(gameObject);
        }
        if(other.tag == "Player")
        {
            PlayerController_V2 playerController = other.getScript<PlayerController_V2>();
            playerController.TakeDamage(damage);
            if (gameObject != null)
                Destroy(gameObject);
        }
    }

}