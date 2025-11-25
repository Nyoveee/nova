// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Gunner : Enemy
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Prefab projectilePrefab;
    [SerializableField]
    private GameObject? projectileSpawnPoint;
    [SerializableField]
    private Rigidbody_? rigidbody;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private enum GunnerState
    {
        Idle,
        Walk,
        Shoot,
        Stagger,
        Death
    }
    private GunnerState gunnerState = GunnerState.Idle;
    private Dictionary<GunnerState, CurrentState> updateState = new Dictionary<GunnerState, CurrentState>();
    GameObject? targetVantagePoint = null;
    int gunShootIndex = 0;
    /***********************************************************
        Components
    ***********************************************************/
    private GunnerStats? gunnerStats = null;
    private GameGlobalReferenceManager gameGlobalReferenceManager = null;
    /**********************************************************************
        Script Functions
    **********************************************************************/
    protected override void init()
    {
        base.init();
        gunnerStats = getScript<GunnerStats>();
        updateState.Add(GunnerState.Idle, Update_Idle);
        updateState.Add(GunnerState.Walk, Update_Walk);
        updateState.Add(GunnerState.Shoot, Update_Shoot);
        updateState.Add(GunnerState.Stagger, Update_Stagger);
        updateState.Add(GunnerState.Death, Update_Death);
        gameGlobalReferenceManager = GameObject.FindWithTag("Game Global Reference Manager").getScript<GameGlobalReferenceManager>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        updateState[gunnerState]();
    }
    /***********************************************************
       Helpers 
    ***********************************************************/
    private bool HasLineOfSightToPlayer(GameObject from)
    {
        string[] layerMask = { "Wall" };
        float distance = Vector3.Distance(from.transform.position, player.transform.position);
        return PhysicsAPI.Linecast(from.transform.position, player.transform.position, layerMask) == null;
    }
    private void GetVantagePoint()
    {
        targetVantagePoint = null;
        float closestVantagePoint = Single.MaxValue;
        foreach (GameObject vantagePoint in gameGlobalReferenceManager.vantagePoints)
        {
            if (!HasLineOfSightToPlayer(vantagePoint))
                continue;
            float distance = Vector3.Distance(vantagePoint.transform.position, gameObject.transform.position);
            if (distance < closestVantagePoint)
            {
                targetVantagePoint = vantagePoint;
                closestVantagePoint = distance;
            }
        }
    }
    /***********************************************************
       Inherited Functions
    ***********************************************************/
    public override void TakeDamage(float damage)
    {
        gunnerStats.health -= damage;
        if (gunnerStats.health <= 0)
        {
            if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
                SpawnIchor();
            gunnerState = GunnerState.Death;
            animator.PlayAnimation("Gunner_Death");
            NavigationAPI.stopAgent(gameObject);
            rigidbody.enable = false;
        }   
        if (gunnerState == GunnerState.Death || WasRecentlyDamaged())
            return;
        SpawnIchor();
        TriggerRecentlyDamageCountdown();
        AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        Invoke(() =>
        {
            renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
        }, gunnerStats.hurtDuration);
        gunnerState = GunnerState.Stagger;
        animator.PlayAnimation("Gunner_Stagger");
    }
    public override bool IsEngagedInBattle()
    {
        return gunnerState != GunnerState.Idle && gunnerState != GunnerState.Death;
    }
    /**********************************************************************
       Enemy States
    **********************************************************************/
    private void Update_Idle()
    {
        if(GetDistanceFromPlayer() <= gunnerStats.shootingRange)
        {
            GetVantagePoint();
            // Walk towards vantage Point
            if (targetVantagePoint != null)
            {
                gunnerState = GunnerState.Walk;
                animator.PlayAnimation("Gunner_Walk");
                MoveToNavMeshPosition(targetVantagePoint.transform.position);
            }
        }
    }
    private void Update_Walk()
    {
        if (!HasLineOfSightToPlayer(targetVantagePoint))
        {
            GetVantagePoint();
            if (targetVantagePoint == null)
            {
                gunnerState = GunnerState.Idle;
                animator.PlayAnimation("Gunner_Idle");
                NavigationAPI.stopAgent(gameObject);
                return;
            }
            MoveToNavMeshPosition(targetVantagePoint.transform.position);
        }
        if(Vector3.Distance(targetVantagePoint.transform.position, gameObject.transform.position) <= gunnerStats.targetDistanceFromVantagePoint)
        {
            gunnerState = GunnerState.Shoot;
            animator.PlayAnimation("Gunner_Attack");
            NavigationAPI.stopAgent(gameObject);
            return;
        }
        LookAtObject(targetVantagePoint);
    }
    private void Update_Shoot()
    {
        LookAtPlayer();
        if (!HasLineOfSightToPlayer(targetVantagePoint))
        {
            gunnerState = GunnerState.Idle;
            animator.PlayAnimation("Gunner_Idle");
            return;
        }
    }
    private void Update_Stagger()
    {

    }
    private void Update_Death(){}
    /**********************************************************************
       Animation Events
    **********************************************************************/
    public void Shoot()
    {
        AudioAPI.PlaySound(gameObject, "Gun1_LaserRifle_Switch_Select5");
        gunShootIndex = (gunShootIndex + 1) % 2;
        AudioAPI.PlaySound(gameObject, gunShootIndex == 0 ? "LaserRifle_SmallRocket_Shot1" : "LaserRifle_SmallRocket_Shot2");
        // Shoot Projectile
        GameObject projectile = Instantiate(projectilePrefab);
        projectile.transform.localPosition = projectileSpawnPoint.transform.position;
        Vector3 direction = player.transform.position - projectileSpawnPoint.transform.position;
        direction.Normalize();
        projectile.getScript<GunnerProjectile>().SetDirection(direction);
    }
    public void EndStagger()
    {
        gunnerState = GunnerState.Idle;
        animator.PlayAnimation("Gunner_Idle");
    }
    public void EndDeath()
    {
        Destroy(gameObject);
    }
}