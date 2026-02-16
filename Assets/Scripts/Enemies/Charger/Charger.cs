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
    private GameObject? attackHitbox = null;
    [SerializableField]
    private Prefab stompHitBoxPrefab;
    [SerializableField]
    private Transform_? stompHitBoxTransform;
    [SerializableField]
    private ParticleEmitter_? emitter;
    [SerializableField]
    private GameObject? chargeLines;
    [SerializableField]
    private List<Audio> attackSFX;
    [SerializableField]
    private List<Audio> deathSFX;
    [SerializableField]
    private List<Audio> hurtSFX;
    [SerializableField]
    private List<Audio> impactSFX;
    [SerializableField]
    private List<Audio> spotSFX;
    [SerializableField]
    private List<Audio> footstepSFX;
    [SerializableField]
    private float timeSinceLastFootstep = 0f;
    [SerializableField]
    private Audio stompSFX;
    [SerializableField]
    private Audio landSFX;

    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? chargerstats = null;
    private AudioComponent_ audioComponent;
    [SerializableField]
    private Rigidbody_? rigidbody;
    // Charge
    private float currentChargeTime = 0f;
    private float currentChargeCooldown = 0f;
    private float currentFootStepTime = 0f;
    private int footStepIndex = 0;
    Vector3 chargeDirection;
    // Stomp
    private float currentStompCooldown = 0f;
    private GameObject? stompHitbox;
    // State machine
    private enum ChargerState
    {
        Spawning,
        Idle,
        Walk,
        Charging,
        Attack,
        Stagger,
        Stomp,
        Jump,
        Death
    }
    private ChargerState chargerState = ChargerState.Spawning;
    private Dictionary<ChargerState, CurrentState> updateState = new Dictionary<ChargerState, CurrentState>();
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    protected override void init()
    {
        base.init();
        chargerstats = getScript<ChargerStats>();
        audioComponent = getComponent<AudioComponent_>();
        updateState.Add(ChargerState.Spawning, Update_Spawning);
        updateState.Add(ChargerState.Idle, Update_Idle);
        updateState.Add(ChargerState.Walk, Update_Walk);
        updateState.Add(ChargerState.Charging, Update_Charging);
        updateState.Add(ChargerState.Attack, Update_Attack);
        updateState.Add(ChargerState.Stagger, Update_Stagger);
        updateState.Add(ChargerState.Stomp, Update_Stomp);
        updateState.Add(ChargerState.Jump, Update_Jump);
        updateState.Add(ChargerState.Death, Update_Death);

        ActivateRigidbody();

        if (attackHitbox != null)
        {
            attackHitbox.SetActive(false);
        }

    }

    protected override void update()
    {
        updateState[chargerState]();
        FlushDamageEnemy();
    }
    /***********************************************************
        Helper Functions
    ***********************************************************/
    /***********************************************************
        Inherited Functions
    ***********************************************************/
    public override void TakeDamage(float damage, Enemy.EnemydamageType damageType, string colliderTag)
    {

        if (damageType == Enemy.EnemydamageType.WeaponShot)
        {
            audioComponent.PlayRandomSound(impactSFX);
            if (colliderTag == "Enemy_ArmouredSpot" || colliderTag == "Enemy")
            {
                damage *= chargerstats.enemyArmouredMultiplier;

            }
            if (colliderTag == "Enemy_WeakSpot")
            {
                damage *= chargerstats.enemyWeakSpotMultiplier;

            }

            accumulatedDamageInstance += damage;
            //chargerstats.health -= damage;
            //if (chargerstats.health <= 0)
            //{
            //    if (chargerState != ChargerState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    chargerState = ChargerState.Death;
            //    animator.PlayAnimation("ChargerDeath");
            //    AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
            //    chargingRigidbody.enable = false;
            //    navMeshRigidbody.enable = false;
            //    chargeLines.SetActive(false);
            //    NavigationAPI.stopAgent(gameObject);
            //}


        }

        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (chargerstats.health <= chargerstats.enemyExecuteThreshold)
            {
                Explode();

                chargerState = ChargerState.Death;
                if (gameObject != null)
                    Destroy(gameObject);
            }
            else
            {
                accumulatedDamageInstance += damage;
                //chargerstats.health -= damage;
                //if (chargerstats.health <= 0)
                //{
                //    if (chargerState != ChargerState.Death && !WasRecentlyDamaged())
                //        SpawnIchor();
                //    chargerState = ChargerState.Death;
                //    animator.PlayAnimation("ChargerDeath");
                //    AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
                //    chargingRigidbody.enable = false;
                //    navMeshRigidbody.enable = false;
                //    chargeLines.SetActive(false);
                //    NavigationAPI.stopAgent(gameObject);

                //}
            }
        }


        if (damageType == Enemy.EnemydamageType.Ultimate)
        {
            accumulatedDamageInstance += damage;
            //chargerstats.health -= damage;
            //if (chargerstats.health <= 0)
            //{
            //    if (chargerState != ChargerState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    chargerState = ChargerState.Death;
            //    animator.PlayAnimation("ChargerDeath");
            //    AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
            //    chargingRigidbody.enable = false;
            //    navMeshRigidbody.enable = false;
            //    chargeLines.SetActive(false);
            //    NavigationAPI.stopAgent(gameObject);

            //}


        }
        //    if (chargerState == ChargerState.Death || WasRecentlyDamaged())
        //    return;
        //TriggerRecentlyDamageCountdown();
        //SpawnIchor();
        //renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        //renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
        //Invoke(() =>
        //{
        //    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
        //    renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
        //}, chargerstats.hurtDuration);

    }
    public override bool IsEngagedInBattle()
    {
        return chargerState != ChargerState.Idle;
    }

    void FlushDamageEnemy()
    {
        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();

            chargerstats.health -= accumulatedDamageInstance;
            UpdateExecutableMaterialState();
            if (chargerstats.health <= 0)
            {
                if (chargerState != ChargerState.Death)
                {
                    chargerState = ChargerState.Death;
                    audioComponent.PlayRandomSound(deathSFX);
                    chargeLines.SetActive(false);
                    DisablePhysicalInteraction();
                    animator.PlayAnimation("ChargerDeath");
                }
            }
            else
            {
                TriggerRecentlyDamageCountdown();
                if (chargerState == ChargerState.Death && !WasRecentlyDamaged())
                {
                    audioComponent.PlayRandomSound(hurtSFX);
                    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
                    Invoke(() =>
                    {
                        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
                    }, chargerstats.hurtDuration);
                }
            }
            accumulatedDamageInstance = 0;
        }
    }

    private void HandleFootStep()
    {
        if (chargerState == ChargerState.Charging && rigidbody.GetVelocity != Vector3.Zero)
        {
            timeSinceLastFootstep += Time.V_DeltaTime();
            if (timeSinceLastFootstep >= chargerstats.timeBetweenChargeSteps)
            {
                //audioComponent.PlayRandomSound(footstepSFX);
                timeSinceLastFootstep = 0;
            }
        }
    }

    /***********************************************************
        State
    ***********************************************************/
    private void Update_Spawning()
    {
        if (IsTouchingGround())
        {
            ActivateNavMeshAgent();
            chargerState = ChargerState.Idle;
            gameObject.transform.position = new Vector3(gameObject.transform.position.x, 0, gameObject.transform.position.z);
        }
    }
    private void Update_Idle(){
        currentChargeCooldown -= Time.V_DeltaTime();
        if (GetDistanceFromPlayer() < chargerstats.chasingRange && HasLineOfSightToPlayer(gameObject))
        {
            audioComponent.PlayRandomSound(spotSFX);
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            return;
        }
    }
    private void Update_Walk() {
        if(GetDistanceFromPlayer() > chargerstats.chasingRange || !HasLineOfSightToPlayer(gameObject))
        {
            chargerState = ChargerState.Idle;
            animator.PlayAnimation("ChargerIdle");
            NavigationAPI.stopAgent(gameObject);
            return;
        }
        LookAt(player);
        currentChargeCooldown -= Time.V_DeltaTime();
        currentStompCooldown -= Time.V_DeltaTime();
        if (IsOnNavMeshOfflink())
        {
            NavigationAPI.stopAgent(gameObject);
            LookAt(GetTargetJumpPosition());
            chargerState = ChargerState.Jump;
            navMeshAgent.enable = false;
            animator.PlayAnimation("ChargerJump");
            return;
        }
        if (GetDistanceFromPlayer() > chargerstats.attackRange && GetDistanceFromPlayer() < chargerstats.chargingRange && currentChargeCooldown <=0 )
        {
            chargerState = ChargerState.Charging;
            animator.PlayAnimation("ChargerCharge");
            currentChargeTime = chargerstats.maxChargeTime;
            ActivateRigidbody();
            chargeLines.SetActive(true);
            // Set Velocity
            chargeDirection = player.transform.position - gameObject.transform.position;
            chargeDirection.y = 0;
            chargeDirection.Normalize();
            return;
        }
        if(GetDistanceFromPlayer() <= chargerstats.attackRange && currentStompCooldown <= 0)
        {
            chargerState = ChargerState.Stomp;
            animator.PlayAnimation("ChargerStomp");
            audioComponent.PlayRandomSound(attackSFX);
            audioComponent.PlaySound(stompSFX);
            ActivateRigidbody();
            return;
        }
        if(GetDistanceFromPlayer() < chargerstats.attackRange)
        {
            chargerState = ChargerState.Attack;
            animator.PlayAnimation("ChargerAttack");
            audioComponent.PlayRandomSound(attackSFX);
            ActivateNavMeshAgent();
            return;
        }
        // Move Enemy 
        MoveToNavMeshPosition(player.transform.position);
    }
    private void Update_Charging() {
        currentChargeTime -= Time.V_DeltaTime();
        if (currentChargeTime <= 0)
        {
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("ChargerWalk");
            currentChargeCooldown = chargerstats.chargeCooldown;
            ActivateNavMeshAgent();
            chargeLines.SetActive(false);
            return;
        }
        // FootSteps
        currentFootStepTime -= Time.V_DeltaTime();
        if(currentFootStepTime <= 0)
        {
            currentFootStepTime = chargerstats.timeBetweenChargeSteps;
            footStepIndex = (footStepIndex + 1) % 2;
            audioComponent.PlayRandomSound(footstepSFX);
            HandleFootStep();
        }
        physicsRigidbody.SetVelocity(chargeDirection * chargerstats.movementSpeed * chargerstats.chargeSpeedMultiplier + new Vector3(0, physicsRigidbody.GetVelocity().y, 0));
    }
    private void Update_Attack()
    {
        currentChargeCooldown -= Time.V_DeltaTime();
    }
    private void Update_Jump()
    {
        if (IsJumpFinished())
        {
            chargerState = ChargerState.Idle;
            animator.PlayAnimation("ChargerIdle");
            navMeshAgent.CompleteOffMeshLink();
            navMeshAgent.enable = true;
        }
    }
    private void Update_Stagger() { }
    private void Update_Stomp() { }
    private void Update_Death()
    {
        if (IsCurrentlyJumping() && IsJumpFinished())
        {
            navMeshAgent.CompleteOffMeshLink();
            navMeshAgent.enable = true;
        }
            
    }
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
        //attackHitbox = Instantiate(chargerAttackHitBoxPrefab,attackHitBoxTransform.localPosition);
        //attackHitbox.getScript<EnemyHitBox>().SetDamage(chargerstats.damage);

       Debug.Log("Rb Hitbox Activated: " + physicsRigidbody.enable);

       

        if (attackHitbox != null)
        {
            //Debug.Log("Attack Hitbox Activated");
            attackHitbox.SetActive(true);
            attackHitbox.getScript<EnemyHitBox>().ResetValues();
            attackHitbox.getScript<EnemyHitBox>().SetDamage(chargerstats.damage);
        }
    }
    public void EndSwing()
    {
        attackHitbox.SetActive(false);
        //Destroy(attackHitbox);
    }
    public void EndAttack()
    {
        chargerState = ChargerState.Walk;
        animator.PlayAnimation("ChargerWalk");
        ActivateNavMeshAgent();
    }
    public void EndStompAnimation()
    {
        stompHitbox = Instantiate(stompHitBoxPrefab,stompHitBoxTransform.position);
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

    public override void SetSpawningDuration(float seconds)
    {
        chargerState = ChargerState.Spawning;

        Invoke(() =>
        {
            chargerState = ChargerState.Idle;
        }, seconds);
    }

    /***********************************************************
        Collision Events
    ***********************************************************/
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player" && chargerState == ChargerState.Charging)
        {
            PlayerController_V2 playerController = other.getScript<PlayerController_V2>();
            playerController.TakeDamage(chargerstats.chargeDamage);
            chargerState = ChargerState.Attack;
            animator.PlayAnimation("ChargerAttack");
            currentChargeCooldown = chargerstats.chargeCooldown;
            ActivateNavMeshAgent();
            chargeLines.SetActive(false);
        }

        //change to use physics layers

        //if (other.get == "Wall" && chargerState == ChargerState.Charging)
        //{
        //    chargerState = ChargerState.Stagger;
        //    animator.PlayAnimation("ChargerStagger");
        //    audioComponent.PlayRandomSound(hurtSFX);
        //    ActivateNavMeshAgent();
        //    chargeLines.SetActive(false);
        //}

        if ((other.getComponent<Rigidbody_>().GetLayerName() == "Wall" || other.getComponent<Rigidbody_>().GetLayerName() == "Props") && chargerState == ChargerState.Charging)
        {
            chargerState = ChargerState.Stagger;
            animator.PlayAnimation("ChargerStagger");
            audioComponent.PlayRandomSound(hurtSFX);
            ActivateNavMeshAgent();
            chargeLines.SetActive(false);
        }
    }

}