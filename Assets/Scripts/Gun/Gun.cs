// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using System;

public abstract class Gun : Script
{
    public int currentAmmo;
    public int currentSp;
    public int maxAmmo;
    public int maxSp;




    private GameObject player;

    protected override void init()
    {
        maxAmmo = currentAmmo;
        player = GameObject.FindWithTag("Player");
    }

    public abstract bool Fire();

    // This function does a ray cast and checks if it collides with any Enemy.
    // Returns true if it hits an enemy.. you may wanna do something with it?
    public bool RayCastFire(Vector3 position, Vector3 direction, float range, float damage)
    {
        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall"};
        // Raycast..
        RayCastResult? result = PhysicsAPI.Raycast(position, direction, range, mask);

        if (result == null)
        {
            return false; 
        }

        GameObject collidedEntity = new GameObject(result.Value.entity);

        if (collidedEntity.tag != "Wall" && collidedEntity.tag != "Enemy_ArmouredSpot" && collidedEntity.tag != "Enemy_WeakSpot" && collidedEntity.tag != "Enemy")
        {
            return false;
        }

        EnemyCollider enemyColliderScript = collidedEntity.getScript<EnemyCollider>();

        if (enemyColliderScript != null)
        {
            enemyColliderScript.OnColliderShot(damage,Enemy.EnemydamageType.WeaponShot,collidedEntity.tag);
            return true;
        }

        return false;
    }
}