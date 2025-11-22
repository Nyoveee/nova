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
    private Transform_? attackHitBoxTransform;
    [SerializableField]
    private Rigidbody_? chargingRigidbody;
    [SerializableField]
    private Rigidbody_? navMeshRigidbody;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? chargerstats = null;
    private float currentChargeTime = 0f;
    private float currentChargeCooldown = 0f;
    private float currentFootStepTime = 0f;
    private GameObject? hitbox;
    private int footStepIndex = 0;
    // State machine
    private enum ChargerState
    {
        Idle,
        Walk,
        Charging,
        Attack,
        Stagger,
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
        if (chargerState == ChargerState.Death)
            return;
        SpawnIchor();
        chargerstats.health -= damage;
        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
        Invoke(() =>
        {
            renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
            renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
        }, hurtDuration);
        if(chargerstats.health <= 0)
        {
            chargerState = ChargerState.Death;
            animator.PlayAnimation("ChargerDeath");
            AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
            NavigationAPI.stopAgent(gameObject);
            chargingRigidbody.SetVelocity(Vector3.Zero());
        }
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
        if (GetDistanceFromPlayer() > chargerstats.attackRange && GetDistanceFromPlayer() < chargerstats.chargingRange && currentChargeCooldown <=0 )
        {
            chargerState = ChargerState.Charging;
            animator.PlayAnimation("ChargerCharge");
            currentChargeTime = chargerstats.maxChargeTime;
            ActivateRigidbody();
            // Set Velocity
            Vector3 direction = player.transform.position - gameObject.transform.position;
            direction.y = 0;
            direction.Normalize();
            currentChargeCooldown -= Time.V_FixedDeltaTime();
            Debug.Log(direction * chargerstats.movementSpeed * chargerstats.speedMultiplier + new Vector3(0, chargingRigidbody.GetVelocity().y, 0));
            chargingRigidbody.SetVelocity(direction * chargerstats.movementSpeed * chargerstats.speedMultiplier + new Vector3(0, chargingRigidbody.GetVelocity().y, 0));
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
    }
    private void Update_Attack()
    {
        currentChargeCooldown -= Time.V_FixedDeltaTime();
    }
    private void Update_Stagger() { }
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
        hitbox = Instantiate(chargerAttackHitBoxPrefab,attackHitBoxTransform.position,null);
        hitbox.getScript<EnemyHitBox>().SetDamage(chargerstats.damage);
    }
    public void EndSwing()
    {
        Destroy(hitbox);
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