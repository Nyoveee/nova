// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class BulletScript : Script
{
    // 
    public float lifeTime = 10f;
    public float damage = 20f;

    private float timeElapsed = 0;
    private Rigidbody_? rigidbody;
    private Transform_? transform;

    private bool hasCollided = false;
    private GameObject collidedEntity = null;

    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (hasCollided)
        {
            Enemy enemyScript = collidedEntity.getScript<Enemy>();
            if (enemyScript != null) {
                enemyScript.TakeDamage(damage);
            }

            ObjectAPI.Destroy(gameObject);
            return;
        }

        // Raycast in front of the bullet from it's velocity..
        Vector3 movingDirection = rigidbody.GetVelocity();
        Vector3 position = transform.position;

        // raycast normalises the directional vector.. so we multiply back   asas
        RayCastResult? result = PhysicsAPI.Raycast(position, movingDirection, movingDirection.Length() * Time.V_FixedDeltaTime(), gameObject);

        if (result.HasValue)
        {
            RayCastResult collision = result.Value;
            collidedEntity = new GameObject(collision.entity);

            if (collidedEntity.tag == "Wall" || collidedEntity.tag == "Enemy")
            { 
                // we delay object destruction by 1 frame.. since raycast cast forward..
                hasCollided = true;
            }
        }

        if (timeElapsed > lifeTime)
        {
            ObjectAPI.Destroy(gameObject);
        }
        else
        {
            timeElapsed += Time.V_FixedDeltaTime();
        }
    }
}