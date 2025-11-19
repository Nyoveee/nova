// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GunnerProjectile : Script
{
    [SerializableField]
    private float damage;
    [SerializableField]
    private float bulletSpeed;
    [SerializableField]
    private float lifetime;
    private Vector3 direction;
    public void SetDirection(Vector3 direction)
    {
        this.direction = direction;
    }
    protected override void update()
    {
        getComponent<Rigidbody_>().SetVelocity(direction * bulletSpeed);
        lifetime -= Time.V_FixedDeltaTime();
        if (lifetime <= 0)
        {
            Destroy(gameObject);
            return;
        }
    }
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Wall")
        {
            Destroy(gameObject);
        }
        if(other.tag == "Player")
        {
            PlayerController playerController = other.getScript<PlayerController>();
            playerController.TakeDamage(damage);
            Destroy(gameObject);
        }
    }

}