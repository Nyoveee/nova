// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class Grunt : Enemy
{
    private delegate void CurrentState();

    /***********************************************************
        Inspector VariablesF
    ***********************************************************/
    [SerializableField]
    private ParticleEmitter_ emitter = null;
    [SerializableField]
    private GameObject headPosition = null;
    //[SerializableField]
    //private Prefab? hitboxPrefab = null; //NOTE: Prefab has been change to socket collider
    [SerializableField]
    private GameObject? attackHitbox = null;
    //[SerializableField]
    //private GameObject? hitboxPosition = null;
    [SerializableField]
    private float spawningDuration = 1f;
    [SerializableField]
    private List<Audio> hurtSFX;
    [SerializableField]
    private List<Audio> impactSFX;
    [SerializableField]
    private List<Audio> attackSFX;
    [SerializableField]
    private List<Audio> deathSFX;
    [SerializableField]
    private List<Audio> spotSFX;
    [SerializableField]
    private List<Audio> footstepSFX;
    [SerializableField]
    private float timeSinceLastFootstep = 0f;
    
    /***********************************************************
        Components
    ***********************************************************/

    private GruntStats? gruntStats = null;
    private AudioComponent_ audioComponent;

    /***********************************************************
        Runtime variables..
    ***********************************************************/
    private enum GruntState
    {
        Spawning,
        Idle,
        Patrol,
        Chasing,
        Attacking,
        PreJump,
        Jump,
        Stagger,
        Death
    }
    // State machine
    private GruntState gruntState = GruntState.Spawning;
    private GruntState gruntStateBeforeJumping;
    private Dictionary<GruntState, CurrentState> updateState = new Dictionary<GruntState, CurrentState>();
    Vector3 lastKnownPlayerPosition;
    // This function is first invoked when game starts.
    protected override void init()
    {
        base.init();
        //gameObject.transform.rotation = Quaternion.Identity();
        gruntStats = getScript<GruntStats>();
        audioComponent = getComponent<AudioComponent_>();
 
        // Populate state machine dispatcher..
        updateState.Add(GruntState.Spawning, Update_Spawning);
        updateState.Add(GruntState.Idle, Update_IdleState);
        updateState.Add(GruntState.Patrol, Update_PatrolState);
        updateState.Add(GruntState.Chasing, Update_ChasingState);
        updateState.Add(GruntState.Attacking, Update_AttackState);
        updateState.Add(GruntState.Death, Update_Death);
        updateState.Add(GruntState.PreJump, Update_PreJump);
        updateState.Add(GruntState.Jump, Update_Jump);
        updateState.Add(GruntState.Stagger, Update_Stagger);

        // animator.PlayAnimation("Grunt Idle (Base)");

        ActivateRigidbody();

        if (attackHitbox != null)
        {
            attackHitbox.SetActive(false);
        }
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        base.update();
        updateState[gruntState]();
        FlushDamageEnemy();
    }
    /**********************************************************************
        Inheritted Functions
    **********************************************************************/
    public override void StaggerMovement()
    {
        if (navMeshAgent.enable && gruntState != GruntState.Stagger)
        {
            base.StaggerMovement();
            GruntState originalState = gruntState;
            gruntState = GruntState.Stagger;
            Invoke(() => { 
                if(gruntState != GruntState.Death)
                    gruntState = originalState; 
            }, movementStaggerTime);
        }
    }
    public override bool IsEngagedInBattle()
    {
        return gruntState != GruntState.Idle;
    }

    public override void TakeDamage(float damage, Enemy.EnemydamageType damageType, string colliderTag)
    {
        if (gruntState == GruntState.Spawning)
        {
            return;
        }

        if (damageType == Enemy.EnemydamageType.WeaponShot)
        {
            audioComponent.PlayRandomSound(impactSFX);

            if (colliderTag == "Enemy_ArmouredSpot")
            {
                damage *= gruntStats.enemyArmouredMultiplier;

            }
            if (colliderTag == "Enemy_WeakSpot")
            {
                damage *= gruntStats.enemyWeakSpotMultiplier;

            }

            accumulatedDamageInstance += damage;
        }

        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (gruntStats.health <= gruntStats.enemyExecuteThreshold)
            {
                Explode();
                gruntState = GruntState.Death;
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

            gruntStats.health -= accumulatedDamageInstance;
            UpdateExecutableMaterialState();
            if (gruntStats.health <= 0)
            {
                if (gruntState != GruntState.Death/* && !WasRecentlyDamaged()*/)
                {
                    gruntState = GruntState.Death;
                    audioComponent.PlayRandomSound(deathSFX);
                    animator.PlayAnimation("Grunt Death");
                    ActivateDeathFlicker();
                    DisablePhysicalInteraction();
                }
            }
            else
            {
                StaggerMovement();
                TriggerRecentlyDamageCountdown();
                if (gruntState != GruntState.Death && !WasRecentlyDamaged())
                {
                    audioComponent.PlayRandomSound(hurtSFX);

                    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
                    renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
                    Invoke(() =>
                    {
                        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
                        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
                    }, gruntStats.hurtDuration); //bug here is this object dies this frame
                }
            }
            accumulatedDamageInstance = 0;
        }
    }

    private void HandleFootStep()
    {
        if (gruntState == GruntState.Chasing && physicsRigidbody.GetVelocity != Vector3.Zero)
        {
            timeSinceLastFootstep += Time.V_DeltaTime();
            if (timeSinceLastFootstep >= gruntStats.timeBetweenSteps)
            {
                audioComponent.PlayRandomSound(footstepSFX);
                timeSinceLastFootstep = 0;
            }
        }
    }

    // kills this gameobject..
    /**********************************************************************
        Enemy States
    **********************************************************************/
    private void Update_Spawning()
    {
        physicsRigidbody.SetLinearDamping(0);
        if (IsTouchingGround())
        {
            ActivateNavMeshAgent();
            gruntState = GruntState.Idle;
        }
    }
    private void Update_IdleState()
    {
        if (player == null || gruntStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found : " + gameObject);
            return;
        }
        if(GetDistanceFromPlayer() <= gruntStats.chasingRadius 
            && HasLineOfSightToPlayer(headPosition)
            && MoveToNavMeshPosition(player.transform.position))
        {
            //roll a float between 0f and 1f, if it falls under SpotChance% play SpotSFX
            if(Random.Range(0, 1) <= this.spotCallSFXChance)
            {
                audioComponent.PlayRandomSound(spotSFX);
            }
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    private void Update_PatrolState()
    {
        if (IsOnNavMeshOfflink())
        {
            gruntStateBeforeJumping = gruntState;
            gruntState = GruntState.PreJump;
            animator.PlayAnimation("Grunt Jump");
            StopNavMeshMovement();
            LookAt(GetTargetJumpPosition());
            return;
        }
        if (GetDistanceFromPlayer() <= gruntStats.chasingRadius 
            && HasLineOfSightToPlayer(headPosition))
        {
            //roll a float between 0f and 1f, if it falls under SpotChance% play SpotSFX
            if (Random.Range(0, 1) <= this.spotCallSFXChance)
            {
                audioComponent.PlayRandomSound(spotSFX);
            }
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
            return;
        }
        if(IsTargetNavigationPositionReached() && !HasLineOfSightToPlayer(headPosition))
        {
            animator.PlayAnimation("Grunt Idle (Base)");
            gruntState = GruntState.Idle;
            StopNavMeshMovement();
        }
    }
    private void Update_ChasingState()
    {
        // Move Enemy 
        if (!MoveToNavMeshPosition(player.transform.position))
        {
            animator.PlayAnimation("Grunt Idle (Base)");
            gruntState = GruntState.Idle;
            return;
        }
        animator.SetFloat("Range", GetDistanceFromPlayer());
        HandleFootStep();
        if (IsOnNavMeshOfflink())
        {
            gruntStateBeforeJumping = gruntState;
            gruntState = GruntState.PreJump;
            animator.PlayAnimation("Grunt Jump");
            StopNavMeshMovement();
            LookAt(GetTargetJumpPosition());
            return;
        }
        if (GetDistanceFromPlayer() > gruntStats.chasingRadius || !HasLineOfSightToPlayer(headPosition))
        {
            gruntState = GruntState.Patrol;
            MoveToNavMeshPosition(player.transform.position);
            return;
        }
        // Change State
        if (GetDistanceFromPlayer() <= gruntStats.attackRadius)
        {
            animator.PlayAnimation("Grunt Attack");
            gruntState = GruntState.Attacking;
            StopNavMeshMovement();
            return;
        }

    }
    private void Update_AttackState()
    {
        if (player == null || gruntStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        LookAt(player);
    }
    private void Update_PreJump() {

    }
    private void Update_Jump()
    {
        if (IsJumpFinished())
        {
            gruntState = gruntStateBeforeJumping;
            animator.PlayAnimation("Grunt Running");
            navMeshAgent.enable = true;
            navMeshAgent.CompleteOffMeshLink();
            ContinueExistingPath();
        }
    }
    private void Update_Stagger() { }
    private void Update_Death(){
        if (IsCurrentlyJumping() && IsJumpFinished())
        {
            navMeshAgent.CompleteOffMeshLink();
            navMeshAgent.enable = true;
        }

    }
    /****************************************************************
        Animation Events
    ****************************************************************/
    public void Slash()
    {
        emitter.emit(1000);
        attackHitbox.SetActive(false);
        //if (hitbox != null)
        //    Destroy(hitbox);
    }
    public void EndAttack()
    {
        if (GetDistanceFromPlayer() > gruntStats.attackRadius)
        {
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    public void BeginSwing()
    {
        audioComponent.PlayRandomSound(attackSFX);
        if (attackHitbox != null)
        {
            //Debug.Log("Attack Hitbox Activated");
            attackHitbox.SetActive(true);
            attackHitbox.getScript<EnemyHitBox>().ResetValues();
            attackHitbox.getScript<EnemyHitBox>().SetDamage(gruntStats.damage);
        }

    }

    public void EndDeath()
    {
    
    }

    public void BeginJump()
    {
        gruntState = GruntState.Jump;
        navMeshAgent.enable = false;
    }
    // ------------
    public override void SetSpawningDuration(float seconds)
    {
        gruntState = GruntState.Spawning;


        Invoke(() =>
        {
            gruntState = GruntState.Idle;
        }, seconds);
    }

    public bool IsDead()
    {
        return gruntState == GruntState.Death;
    }
}