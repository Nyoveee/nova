// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public abstract class Gun : Script
{
    [SerializableField]
    private GameUIManager gameUIManager;

    [SerializableField]
    private int initialMaxAmmo;

    [SerializableField]
    private int initialCurrentSp = 0;

    [SerializableField]
    private int initialMaxSp = 30;

    // Private backing field
    private int currentAmmo;

    // Public property with get and set accessors
    public int CurrentAmmo
    {
        get
        {
            return currentAmmo;
        }
        set
        {
            currentAmmo = value;
            gameUIManager?.SetCurrentAmmoText(currentAmmo);
        }
    }

    // Private backing field
    private int maxAmmo;

    // Public property with get and set accessors. This is how we gurantee sync between ammo and ui.
    public int MaxAmmo
    {
        get
        {
            return maxAmmo;
        }
        set
        {
            maxAmmo = value;
            gameUIManager?.SetAmmoText(currentAmmo, maxAmmo);
        }
    }


    private int currentSp;

    // Public property with get and set accessors
    public int CurrentSp
    {
        get
        {
            return currentSp;
        }
        set
        {
            currentSp = value;
            gameUIManager?.SetUltimateBarUI(currentSp, maxSp);
        }
    }

    // Private backing field
    private int maxSp;

    // Public property with get and set accessors. This is how we gurantee sync between ammo and ui.
    public int MaxSp
    {
        get
        {
            return maxSp;
        }
        set
        {
            maxSp = value;
            gameUIManager?.SetUltimateBarUI(currentSp, maxSp);
        }
    }

    private GameObject player;

    protected override void init()
    {
        maxAmmo = currentAmmo = initialMaxAmmo;
        maxSp = initialMaxSp;
        currentSp = initialCurrentSp;

        player = GameObject.FindWithTag("Player");
        gameUIManager?.SetAmmoText(currentAmmo, maxAmmo);
        gameUIManager?.SetUltimateBarUI(currentSp, maxSp);
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