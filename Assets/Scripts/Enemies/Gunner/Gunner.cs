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
    private GameObject? gunnerHead;
    [SerializableField]
    private Rigidbody_? rigidBody;
    [SerializableField]
    private List<Audio> attackSFX;
    [SerializableField]
    private List<Audio> impactSFX;
    [SerializableField]
    private List<Audio> deathSFX;
    [SerializableField]
    private List<Audio> spotSFX;
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
    private GunnerState gunnerState = GunnerState.Spawning;
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
        updateState.Add(GunnerState.Spawning, Update_Spawning);
        updateState.Add(GunnerState.Idle, Update_Idle);
        updateState.Add(GunnerState.Walk, Update_Walk);
        updateState.Add(GunnerState.Shoot, Update_Shoot);
        updateState.Add(GunnerState.Stagger, Update_Stagger);
        updateState.Add(GunnerState.PreJump, Update_PreJump);
        updateState.Add(GunnerState.Jump, Update_Jump);
        updateState.Add(GunnerState.Death, Update_Death);

        ActivateRigidbody();

        GameObject gameObject = GameObject.FindWithTag("Game Global Reference Manager");
        gameGlobalReferenceManager = gameObject?.getScript<GameGlobalReferenceManager>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        base.update();
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
        if (gameGlobalReferenceManager == null || gameGlobalReferenceManager.vantagePoints == null)
            return;
        foreach (GameObject vantagePoint in gameGlobalReferenceManager.vantagePoints)
        {
            if (!HasLineOfSightToPlayer(vantagePoint))
                continue;
            // Vantage point must be at a safe distance from the player that the gunner can shoot from
            if (Vector3.Distance(vantagePoint.transform.position, playerHead.transform.position) <= gunnerStats.safeVantageRange)
                continue;
            float distance = Vector3.Distance(vantagePoint.transform.position, gunnerHead.transform.position);
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
    public override void StaggerMovement()
    {
        if (navMeshAgent.enable && gunnerState!= GunnerState.Stagger)
        {
            base.StaggerMovement();
            GunnerState originalState = gunnerState;
            gunnerState = GunnerState.Stagger;
            Invoke(() => {
                if (gunnerState != GunnerState.Death)
                    gunnerState = originalState;
                if(gunnerState == GunnerState.Walk)
                    MoveToNavMeshPosition(targetVantagePoint.transform.position);
            }, movementStaggerTime);
        }
    }
    public override void TakeDamage(float damage, Enemy.EnemydamageType damageType, string colliderTag)
    {
        audioComponent.PlayRandomSound(impactSFX);

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
        }


        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (gunnerStats.health <= gunnerStats.enemyExecuteThreshold)
            {
                Explode();

                gunnerState = GunnerState.Death;
                if (gameObject != null)
                    Destroy(gameObject);
            }
            else 
            {

                accumulatedDamageInstance += damage;
            }
        }

        if (damageType == Enemy.EnemydamageType.Ultimate)
        {

            accumulatedDamageInstance += damage;
        }

        accumulatedDamageInstance = MathF.Min(accumulatedDamageInstance, enemyStats.health);
    }

    void FlushDamageEnemy()
    {
        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();

            gunnerStats.health -= accumulatedDamageInstance;
            UpdateExecutableMaterialState();
            if (gunnerStats.health <= 0)
            {
                if (gunnerState != GunnerState.Death/* && !WasRecentlyDamaged()*/)
                {
                    gunnerState = GunnerState.Death;
                    audioComponent.PlayRandomSound(deathSFX);
                    animator.PlayAnimation("Gunner_Death");
                    ActivateDeathFlicker();
                    DisablePhysicalInteraction();
                }
            }
            else
            {
                StaggerMovement();
                TriggerRecentlyDamageCountdown();
                if (gunnerState != GunnerState.Death && !WasRecentlyDamaged())
                {
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
    private void Update_Spawning()
    {
        physicsRigidbody.SetLinearDamping(0);
        if (IsTouchingGround())
        {
            ActivateNavMeshAgent();
            gunnerState = GunnerState.Idle;
        }
    }
    private void Update_Idle()
    {
        GetVantagePoint();

        // Walk towards vantage Point
        if (targetVantagePoint != null)
        {
            audioComponent.PlayRandomSound(spotSFX);
            gunnerState = GunnerState.Walk;
            animator.PlayAnimation("Gunner_Run");
            MoveToNavMeshPosition(targetVantagePoint.transform.position);
        }
    }
    private void Update_Walk()
    {
        if (!HasLineOfSightToPlayer(targetVantagePoint) || Vector3.Distance(targetVantagePoint.transform.position, playerHead.transform.position) <= gunnerStats.escapeRange)
        {
            GetVantagePoint();

            if (targetVantagePoint == null)
            {
                gunnerState = GunnerState.Idle;
                animator.PlayAnimation("Gunner_Idle");
                StopNavMeshMovement();
                return;
            }
            MoveToNavMeshPosition(targetVantagePoint.transform.position);
        }
        if (IsOnNavMeshOfflink())
        {
            gunnerState = GunnerState.PreJump;
            animator.PlayAnimation("Gunner_Jump");
            StopNavMeshMovement();
            LookAt(GetTargetJumpPosition());
            return;
        }

        Vector3 vantagePoint = new Vector3(targetVantagePoint.transform.position.x, 0, targetVantagePoint.transform.position.z);
        Vector3 gunnerPos     = new Vector3(gameObject.transform.position.x,         0, gameObject.transform.position.z);
      
        if(HasLineOfSightToPlayer(gunnerHead) && GetDistanceFromPlayer() >= gunnerStats.shootingRange)
        {
            gunnerState = GunnerState.Shoot;
            animator.PlayAnimation("Gunner_Attack");
            StopNavMeshMovement();
        }
        else if (Vector3.Distance(vantagePoint, gunnerPos) <= gunnerStats.targetDistanceFromVantagePoint)
        {
            gunnerState = GunnerState.Shoot;
            animator.PlayAnimation("Gunner_Attack");
            StopNavMeshMovement();
        }
    }
    private void Update_Shoot()
    {
        LookAt(player);
        if (!HasLineOfSightToPlayer(gunnerHead) || GetDistanceFromPlayer() < gunnerStats.escapeRange)
        {
            gunnerState = GunnerState.Idle;
            animator.PlayAnimation("Gunner_Idle");
            return;
        }
    }
    private void Update_Stagger(){
    
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
        //audioComponent.PlayRandomSound(attackSFX);
        gunShootIndex = (gunShootIndex + 1) % 2;
        // Shoot Projectile
        GameObject projectile = Instantiate(projectilePrefab);
        projectile.transform.position = projectileSpawnPoint.transform.position;
        Vector3 direction = playerHead.transform.position - projectileSpawnPoint.transform.position;
        direction.Normalize();
        projectile.getScript<GunnerProjectile>().SetDirection(direction);
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