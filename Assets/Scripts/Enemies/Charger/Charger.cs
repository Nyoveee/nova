// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.CompilerServices;

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
    private float hurtDuration = 0.1f;
    [SerializableField]
    private Material? defaultMaterial = null;
    [SerializableField]
    private Material? defaultMaterial2 = null;
    [SerializableField]
    private Material? hurtMaterial = null;
    [SerializableField]
    private Material? hurtMaterial2 = null;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? chargerstats = null;
    private float currentChargeTime = 0f;
    private float currentChargeCooldown = 0f;
    private float currentFootStepTime = 0f;
    private float currentHurtTime = 0f;
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
    }
    protected override void update() {
        if(currentHurtTime > 0)
        {
            currentHurtTime -= Time.V_FixedDeltaTime();
            if(currentHurtTime <= 0)
            {
                renderer.changeMaterial(0, defaultMaterial);
                renderer.changeMaterial(1, defaultMaterial2);
            }
        }
        updateState[chargerState](); 
    }
    /***********************************************************
       Public Functions
    ***********************************************************/
    public bool IsCharging() { return chargerState == ChargerState.Charging; }
    /***********************************************************
        Inherited Functions
    ***********************************************************/
    public override void TakeDamage(float damage)
    {
        if (chargerState == ChargerState.Death)
            return;
        chargerstats.health -= damage;
        currentHurtTime = hurtDuration;
        renderer.changeMaterial(0, hurtMaterial);
        renderer.changeMaterial(1, hurtMaterial2);
        if(chargerstats.health <= 0)
        {
            chargerState = ChargerState.Death;
            animator.PlayAnimation("ChargerDeath");
            rigidbody.SetVelocity(Vector3.Zero());
            AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
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
            rigidbody.SetVelocity(Vector3.Zero());
            return;
        }
        LookAtPlayer();
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        currentChargeCooldown -= Time.V_FixedDeltaTime();
        if (GetDistanceFromPlayer() > chargerstats.attackRange && GetDistanceFromPlayer() < chargerstats.chargingRange && currentChargeCooldown <=0 )
        {
            chargerState = ChargerState.Charging;
            animator.PlayAnimation("ChargerCharge");
            currentChargeTime = chargerstats.maxChargeTime;
            rigidbody.SetVelocity(direction * chargerstats.movementSpeed * chargerstats.speedMultiplier + new Vector3(0, rigidbody.GetVelocity().y, 0));
            return;
        }
        if(GetDistanceFromPlayer() < chargerstats.attackRange)
        {
            chargerState = ChargerState.Attack;
            animator.PlayAnimation("ChargerAttack");
            rigidbody.SetVelocity(Vector3.Zero());
            return;
        }
        // Move Enemy 
        rigidbody.SetVelocity(direction * chargerstats.movementSpeed + new Vector3(0, rigidbody.GetVelocity().y, 0));
    }
    private void Update_Charging() {
        currentChargeTime -= Time.V_FixedDeltaTime();
        if (currentChargeTime <= 0)
        {
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            currentChargeCooldown = chargerstats.chargeCooldown;
        }
        // FootSteps
        currentFootStepTime -= Time.V_FixedDeltaTime();
        if(currentFootStepTime <= 0)
        {
            currentFootStepTime = chargerstats.timeBetweenChargeSteps;
            footStepIndex = (footStepIndex + 1)%2;
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
            rigidbody.SetVelocity(Vector3.Zero());
            currentChargeCooldown = chargerstats.chargeCooldown;
        }
        if (other.tag == "Wall" && chargerState == ChargerState.Charging)
        {
            chargerState = ChargerState.Stagger;
            animator.PlayAnimation("ChargerStagger");
            rigidbody.SetVelocity(Vector3.Zero());
            AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        }
    }
}