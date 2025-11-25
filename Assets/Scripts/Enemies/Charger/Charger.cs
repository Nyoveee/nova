// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class Charger : Enemy
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Prefab chargerAttackHitBoxPrefab;
    [SerializableField]
    private Prefab stompHitBoxPrefab;
    [SerializableField]
    private Transform_? attackHitBoxTransform;
    [SerializableField]
    private Transform_? stompHitBoxTransform;
    [SerializableField]
    private Rigidbody_? chargingRigidbody;
    [SerializableField]
    private Rigidbody_? navMeshRigidbody;
    [SerializableField]
    private ParticleEmitter_? emitter;

    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? chargerstats = null;
    // Charge
    private float currentChargeTime = 0f;
    private float currentChargeCooldown = 0f;
    private float currentFootStepTime = 0f;
    private int footStepIndex = 0;
    Vector3 chargeDirection;
    // Attack
    private GameObject? attackHitbox;
    // Stomp
    private float currentStompCooldown = 0f;
    private GameObject? stompHitbox;
    // State machine
    private enum ChargerState
    {
        Idle,
        Walk,
        Charging,
        Attack,
        Stagger,
        Stomp,
        Death
    }
    private ChargerState chargerState = ChargerState.Idle;
    private Dictionary<ChargerState, CurrentState> updateState = new Dictionary<ChargerState, CurrentState>();
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    protected override void init()
    {
        base.init();
        chargerstats = getScript<ChargerStats>();
        updateState.Add(ChargerState.Idle, Update_Idle);
        updateState.Add(ChargerState.Walk, Update_Walk);
        updateState.Add(ChargerState.Charging, Update_Charging);
        updateState.Add(ChargerState.Attack, Update_Attack);
        updateState.Add(ChargerState.Stagger, Update_Stagger);
        updateState.Add(ChargerState.Stomp, Update_Stomp);
        updateState.Add(ChargerState.Death, Update_Death);
        ActivateNavMeshAgent();
    }
    protected override void update() {
        updateState[chargerState](); 
    }
    /***********************************************************
        Helper Functions
    ***********************************************************/
    private void ActivateRigidbody()
    {
        navMeshAgent.enable = false;
        chargingRigidbody.enable = true;
        navMeshRigidbody.enable = false;
        NavigationAPI.stopAgent(gameObject);
    }
    private void ActivateNavMeshAgent()
    {
        navMeshAgent.enable = true;
        chargingRigidbody.enable = false;
        navMeshRigidbody.enable = true;
        chargingRigidbody.SetVelocity(Vector3.Zero());
    }
    /***********************************************************
        Inherited Functions
    ***********************************************************/
    public override void TakeDamage(float damage)
    {
        chargerstats.health -= damage;
        if (chargerstats.health <= 0)
        {
            if (chargerState != ChargerState.Death && !WasRecentlyDamaged())
                SpawnIchor();
            chargerState = ChargerState.Death;
            animator.PlayAnimation("ChargerDeath");
            AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
            NavigationAPI.stopAgent(gameObject);
            chargingRigidbody.enable = false;
            navMeshRigidbody.enable = false;
            navMeshAgent.enable = false;
        }
        if (chargerState == ChargerState.Death || WasRecentlyDamaged())
            return;
        TriggerRecentlyDamageCountdown();
        SpawnIchor();
        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
        Invoke(() =>
        {
            renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
            renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
        }, chargerstats.hurtDuration);
        
    }
    public override bool IsEngagedInBattle()
    {
        return chargerState != ChargerState.Idle && chargerState != ChargerState.Death;
    }
    /***********************************************************
        State
    ***********************************************************/
    private void Update_Idle(){
        currentChargeCooldown -= Time.V_FixedDeltaTime();
        if (GetDistanceFromPlayer() < chargerstats.chasingRange)
        {
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            return;
        }
    }
    private void Update_Walk() {
        if(GetDistanceFromPlayer() > chargerstats.chasingRange)
        {
            chargerState = ChargerState.Idle;
            animator.PlayAnimation("ChargerIdle");
            NavigationAPI.stopAgent(gameObject);
            return;
        }
        LookAtPlayer();
        currentChargeCooldown -= Time.V_FixedDeltaTime();
        currentStompCooldown -= Time.V_FixedDeltaTime();
        if (GetDistanceFromPlayer() > chargerstats.attackRange && GetDistanceFromPlayer() < chargerstats.chargingRange && currentChargeCooldown <=0 )
        {
            chargerState = ChargerState.Charging;
            animator.PlayAnimation("ChargerCharge");
            currentChargeTime = chargerstats.maxChargeTime;
            ActivateRigidbody();
            // Set Velocity
            chargeDirection = player.transform.position - gameObject.transform.position;
            chargeDirection.y = 0;
            chargeDirection.Normalize();
            return;
        }
        if(GetDistanceFromPlayer() <= chargerstats.attackRange && currentStompCooldown <= 0)
        {
            chargerState = ChargerState.Stomp;
            animator.PlayAnimation("ChargerJump");
            ActivateRigidbody();
            return;
        }
        if(GetDistanceFromPlayer() < chargerstats.attackRange)
        {
            chargerState = ChargerState.Attack;
            animator.PlayAnimation("ChargerAttack");
            ActivateRigidbody();
            chargingRigidbody.SetVelocity(Vector3.Zero());
            return;
        }
        // Move Enemy 
        MoveToNavMeshPosition(player.transform.position);
    }
    private void Update_Charging() {
        currentChargeTime -= Time.V_FixedDeltaTime();
        if (currentChargeTime <= 0)
        {
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            currentChargeCooldown = chargerstats.chargeCooldown;
            ActivateNavMeshAgent();
            return;
        }
        // FootSteps
        currentFootStepTime -= Time.V_FixedDeltaTime();
        if(currentFootStepTime <= 0)
        {
            currentFootStepTime = chargerstats.timeBetweenChargeSteps;
            footStepIndex = (footStepIndex + 1) % 2;
            AudioAPI.PlaySound(gameObject, footStepIndex == 0 ? "sfx_enemyChargeStep_01mono" : "sfx_enemyChargeStep_02mono");
        }
        chargingRigidbody.SetVelocity(chargeDirection * chargerstats.movementSpeed * chargerstats.chargeSpeedMultiplier + new Vector3(0, chargingRigidbody.GetVelocity().y, 0));
    }
    private void Update_Attack()
    {
        currentChargeCooldown -= Time.V_FixedDeltaTime();
    }
    private void Update_Stagger() { }
    private void Update_Stomp() { }
    private void Update_Death() { }
    /***********************************************************
        Animation Events
    ***********************************************************/
    public void EndStagger()
    {
        chargerState = ChargerState.Walk;
        animator.PlayAnimation("ChargerWalk");
        currentChargeCooldown = chargerstats.chargeCooldown;
        ActivateNavMeshAgent();
    }
    public void BeginSwing()
    {
        attackHitbox = Instantiate(chargerAttackHitBoxPrefab,attackHitBoxTransform.localPosition);
        attackHitbox.getScript<EnemyHitBox>().SetDamage(chargerstats.damage);
    }
    public void EndSwing()
    {
        Destroy(attackHitbox);
    }
    public void EndAttack()
    {
        chargerState = ChargerState.Walk;
        animator.PlayAnimation("ChargerWalk");
        ActivateNavMeshAgent();
    }
    public void EndDeathAnimation()
    {
        Destroy(gameObject);
    }
    public void EndStompAnimation()
    {
        stompHitbox = Instantiate(stompHitBoxPrefab);
        emitter.emit(30);
        stompHitbox.getScript<EnemyHitBox>().SetDamage(chargerstats.stompDamage);
        Invoke(() =>
        {
            Destroy(stompHitbox);
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            ActivateNavMeshAgent();
            currentStompCooldown = chargerstats.stompCooldown;
        }, chargerstats.stompHitboxDuration);
    }
    /***********************************************************
        Collision Events
    ***********************************************************/
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player" && chargerState == ChargerState.Charging)
        {
            PlayerController playerController = other.getScript<PlayerController>();
            playerController.TakeDamage(chargerstats.chargeDamage);
            chargerState = ChargerState.Attack;
            animator.PlayAnimation("ChargerAttack");
            currentChargeCooldown = chargerstats.chargeCooldown;
            chargingRigidbody.SetVelocity(Vector3.Zero());
        }
        if (other.tag == "Wall" && chargerState == ChargerState.Charging)
        {
            chargerState = ChargerState.Stagger;
            animator.PlayAnimation("ChargerStagger");
            AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
            chargingRigidbody.SetVelocity(Vector3.Zero());
        }
    }
}