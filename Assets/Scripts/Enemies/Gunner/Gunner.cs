// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Threading.Tasks;

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
    private Rigidbody_? rigidBody;
    [SerializableField]
    private List<Audio> attackSFX;
    [SerializableField]
    private List<Audio> deathSFX;
    [SerializableField]
    private List<Audio> footstepSFX;
    [SerializableField]
    private float timeSinceLastFootstep = 0f;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private enum GunnerState
    {
        Spawning,
        Idle,
        Walk,
        Shoot,
        Stagger,
        PreJump,
        Jump,
        Death
    }
    private GunnerState gunnerState = GunnerState.Idle;
    private Dictionary<GunnerState, CurrentState> updateState = new Dictionary<GunnerState, CurrentState>();
    GameObject? targetVantagePoint = null;
    int gunShootIndex = 0;
    NavMeshOfflinkData offlinkData;
    /***********************************************************
        Components
    ***********************************************************/
    private GunnerStats? gunnerStats = null;
    private GameGlobalReferenceManager gameGlobalReferenceManager = null;
    private AudioComponent_ audioComponent;
    /**********************************************************************
        Script Functions
    **********************************************************************/
    protected override void init()
    {
        base.init();
        gunnerStats = getScript<GunnerStats>();
        audioComponent = getComponent<AudioComponent_>();
        updateState.Add(GunnerState.Idle, Update_Idle);
        updateState.Add(GunnerState.Walk, Update_Walk);
        updateState.Add(GunnerState.Shoot, Update_Shoot);
        updateState.Add(GunnerState.Stagger, Update_Stagger);
        updateState.Add(GunnerState.PreJump, Update_PreJump);
        updateState.Add(GunnerState.Jump, Update_Jump);
        updateState.Add(GunnerState.Death, Update_Death);

        updateState.Add(GunnerState.Spawning, () => { });

        gameGlobalReferenceManager = GameObject.FindWithTag("Game Global Reference Manager").getScript<GameGlobalReferenceManager>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        updateState[gunnerState]();
        FlushDamageEnemy();
    }
    /***********************************************************
       Helpers 
    ***********************************************************/

    private void GetVantagePoint()
    {
        targetVantagePoint = null;
        float closestVantagePoint = Single.MaxValue;
        if (gameGlobalReferenceManager.vantagePoints == null)
            return;
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
    public override void TakeDamage(float damage, Enemy.EnemydamageType damageType, string colliderTag)
    {
        if (damageType == Enemy.EnemydamageType.WeaponShot)
        {
            if (colliderTag == "Enemy_ArmouredSpot")
            {
                damage *= gunnerStats.enemyArmouredMultiplier;

            }
            if (colliderTag == "Enemy_WeakSpot")
            {
                damage *= gunnerStats.enemyWeakSpotMultiplier;

            }

            accumulatedDamageInstance += damage;
            //gunnerStats.health -= damage;
            //if (gunnerStats.health <= 0)
            //{
            //    if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    gunnerState = GunnerState.Death;
            //    animator.PlayAnimation("Gunner_Death");
            //    NavigationAPI.stopAgent(gameObject);
            //    rigidBody.enable = false;
            //}


        }


        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (gunnerStats.health <= gunnerStats.enemyExecuteThreshold)
            {
                Explode();

                gunnerState = GunnerState.Death;
                Destroy(gameObject);
            }
            else 
            {

                accumulatedDamageInstance += damage;
                //gunnerStats.health -= damage;
                //if (gunnerStats.health <= 0)
                //{
                //    if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
                //        SpawnIchor();
                //    gunnerState = GunnerState.Death;
                //    animator.PlayAnimation("Gunner_Death");
                //    NavigationAPI.stopAgent(gameObject);
                //    rigidBody.enable = false;
                //}
            }


        }

        if (damageType == Enemy.EnemydamageType.Ultimate)
        {

            accumulatedDamageInstance += damage;
            //gunnerStats.health -= damage;
            //if (gunnerStats.health <= 0)
            //{
            //    if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    gunnerState = GunnerState.Death;
            //    animator.PlayAnimation("Gunner_Death");
            //    NavigationAPI.stopAgent(gameObject);
            //    rigidBody.enable = false;
            //}

        }



        //    if (gunnerState == GunnerState.Death || WasRecentlyDamaged())
        //    return;
        //SpawnIchor();
        //TriggerRecentlyDamageCountdown();
        //AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        //renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        //Invoke(() =>
        //{
        //    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
        //}, gunnerStats.hurtDuration);
        //// Don't stagger if it's in the middle of a jump
        //if (IsCurrentlyJumping())
        //    return;
        //gunnerState = GunnerState.Stagger;
        //animator.PlayAnimation("Gunner_Stagger");
    }

    void FlushDamageEnemy()
    {



        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();

            gunnerStats.health -= accumulatedDamageInstance;
            if (gunnerStats.health <= 0)
            {
                if (gunnerState != GunnerState.Death/* && !WasRecentlyDamaged()*/)
                {
                    gunnerState = GunnerState.Death;
                    animator.PlayAnimation("Gunner_Death");
                    NavigationAPI.stopAgent(gameObject);
                    rigidBody.enable = false;
                }
            }
            else
            {
                TriggerRecentlyDamageCountdown();
                if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
                {
                    //AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
                    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
                    renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
                    Invoke(() =>
                    {
                        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
                        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
                    }, gunnerStats.hurtDuration); //bug here is this object dies this frame
                }
            }
            accumulatedDamageInstance = 0;
        }
    }
    public override bool IsEngagedInBattle()
    {
        return gunnerState != GunnerState.Idle;
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
        if (IsOnNavMeshOfflink())
        {
            gunnerState = GunnerState.PreJump;
            animator.PlayAnimation("Gunner_Jump");
            NavigationAPI.stopAgent(gameObject);
            LookAt(GetTargetJumpPosition());
            return;
        }
        if(Vector3.Distance(targetVantagePoint.transform.position, gameObject.transform.position) <= gunnerStats.targetDistanceFromVantagePoint)
        {
            gunnerState = GunnerState.Shoot;
            animator.PlayAnimation("Gunner_Attack");
            NavigationAPI.stopAgent(gameObject);
            return;
        }
    }
    private void Update_Shoot()
    {
        LookAt(player);
        if (!HasLineOfSightToPlayer(targetVantagePoint))
        {
            gunnerState = GunnerState.Idle;
            animator.PlayAnimation("Gunner_Idle");
            return;
        }
    }
    private void Update_Stagger(){
        if (IsCurrentlyJumping() && IsJumpFinished())
            navMeshAgent.CompleteOffMeshLink();
    }
    private void Update_PreJump() { }
    private void Update_Jump()
    {
        if (IsJumpFinished()){
            gunnerState = GunnerState.Idle;
            animator.PlayAnimation("Gunner_Idle");
            navMeshAgent.CompleteOffMeshLink();
            navMeshAgent.enable = true;
        }
    }
    private void Update_Death(){
        if (IsCurrentlyJumping() && IsJumpFinished())
        {
            navMeshAgent.CompleteOffMeshLink();
            navMeshAgent.enable = true;
        }
            
    }
    /**********************************************************************
       Animation Events
    **********************************************************************/
    public void Shoot()
    {
        //AudioAPI.PlaySound(gameObject, "Gun1_LaserRifle_Switch_Select5");
        gunShootIndex = (gunShootIndex + 1) % 2;
        // AudioAPI.PlaySound(gameObject, gunShootIndex == 0 ? "LaserRifle_SmallRocket_Shot1" : "LaserRifle_SmallRocket_Shot2");
        // Shoot Projectile
        GameObject projectile = Instantiate(projectilePrefab);
        projectile.transform.position = projectileSpawnPoint.transform.position;
        Vector3 direction = player.transform.position - projectileSpawnPoint.transform.position;
        direction.Normalize();
        projectile.getScript<GunnerProjectile>().SetDirection(direction);
    }
    public void EndStagger()
    {
      
        gunnerState = GunnerState.Idle;
        animator.PlayAnimation("Gunner_Idle");
    }
    public void BeginJump()
    {
        gunnerState = GunnerState.Jump;
        navMeshAgent.enable = false;
    }

    // ----
    public override void SetSpawningDuration(float seconds)
    {
        gunnerState = GunnerState.Spawning;

        Invoke(() =>
        {
            gunnerState = GunnerState.Idle;
        }, seconds);
    }
}