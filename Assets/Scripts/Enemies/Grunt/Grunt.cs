// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class Grunt : Enemy
{
    private delegate void CurrentState();

    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private ParticleEmitter_ emitter = null;

    [SerializableField]
    private Prefab? hitboxPrefab = null;
    [SerializableField]
    private GameObject? hitboxPosition = null;
    [SerializableField]
    private float spawningDuration = 1f;
    [SerializableField]
    private Rigidbody_? rigidbody;
    /***********************************************************
        Components
    ***********************************************************/

    private GruntStats? gruntStats = null;

    /***********************************************************
        Runtime variables..
    ***********************************************************/
    private enum GruntState
    {
        Spawning,
        Idle,
        Chasing,
        Attacking,
        PreJump,
        Jump,
        Death
    }
    // State machine
    private GruntState gruntState = GruntState.Idle;
    private Dictionary<GruntState, CurrentState> updateState = new Dictionary<GruntState, CurrentState>();
    private float spawningTimeElapsed = 0f;
    private GameObject? hitbox = null;
    /***********************************************************
        Inspector Variables
    ***********************************************************/

    // This function is first invoked when game starts.
    protected override void init()
    {
        base.init();
        gameObject.transform.rotation = Quaternion.Identity();
        gruntStats = getScript<GruntStats>();

        animator.PlayAnimation("Grunt Idle (Base)");

        // Populate state machine dispatcher..
        updateState.Add(GruntState.Idle, Update_IdleState);
        updateState.Add(GruntState.Chasing, Update_ChasingState);
        updateState.Add(GruntState.Attacking, Update_AttackState);
        updateState.Add(GruntState.Death, Update_Death);
        updateState.Add(GruntState.PreJump, Update_PreJump);
        updateState.Add(GruntState.Jump, Update_Jump);

        // spawning afk...
        updateState.Add(GruntState.Spawning, () => { });

        LookAt(player);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {   
         updateState[gruntState]();
         FlushDamageEnemy();
    }
    /**********************************************************************
        Inheritted Functions
    **********************************************************************/
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

            if (colliderTag == "Enemy_ArmouredSpot")
            {
                damage *= gruntStats.enemyArmouredMultiplier;

            }
            if (colliderTag == "Enemy_WeakSpot")
            {
                damage *= gruntStats.enemyWeakSpotMultiplier;

            }

            accumulatedDamageInstance += damage;
            //gruntStats.health -= damage;
            //if (gruntStats.health <= 0)
            //{
            //    if (gruntState != GruntState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    gruntState = GruntState.Death;
            //    animator.PlayAnimation("Grunt Death");
            //    NavigationAPI.stopAgent(gameObject);
            //    rigidbody.enable = false;
            //}


        }

        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (gruntStats.health <= gruntStats.enemyExecuteThreshold)
            {
                Explode();
                //animator.PlayAnimation("Grunt Death");
                //NavigationAPI.stopAgent(gameObject);
                //rigidbody.enable = false;
                gruntState = GruntState.Death;
                Destroy(gameObject);

            }
            else
            {
                accumulatedDamageInstance += damage;
                //if (gruntStats.health <= 0)
                //{
                //    if (gruntState != GruntState.Death && !WasRecentlyDamaged())
                //        SpawnIchor();
                //    gruntState = GruntState.Death;
                //    animator.PlayAnimation("Grunt Death");
                //    NavigationAPI.stopAgent(gameObject);
                //    rigidbody.enable = false;
                //}
            }

        }

        if (damageType == Enemy.EnemydamageType.Ultimate)
        {

            accumulatedDamageInstance += damage;
            //gruntStats.health -= damage;
            //if (gruntStats.health <= 0)
            //{
            //    if (gruntState != GruntState.Death && !WasRecentlyDamaged())
            //        SpawnIchor();
            //    gruntState = GruntState.Death;
            //    animator.PlayAnimation("Grunt Death");
            //    NavigationAPI.stopAgent(gameObject);
            //    rigidbody.enable = false;
            //}

        }

        //    // blud already died let him die in peace dont take anymore damage..
        //    if (gruntState == GruntState.Death || WasRecentlyDamaged())
        //    return;
        //SpawnIchor();
        //TriggerRecentlyDamageCountdown();

        //if (gruntState != GruntState.Death)
        //{
        //    AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        //    renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        //    renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
        //    Invoke(() =>
        //    {
        //        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
        //        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
        //    }, gruntStats.hurtDuration); //bug here is this object dies this frame
        //}
    }



    void FlushDamageEnemy()
    {
        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();

            gruntStats.health -= accumulatedDamageInstance;
            if (gruntStats.health <= 0)
            {
                if (gruntState != GruntState.Death/* && !WasRecentlyDamaged()*/)
                {
                    gruntState = GruntState.Death;
                    animator.PlayAnimation("Grunt Death");
                    NavigationAPI.stopAgent(gameObject);
                    rigidbody.enable = false;
                }
            }
            else
            {
                TriggerRecentlyDamageCountdown();
                if (gruntState != GruntState.Death && !WasRecentlyDamaged())
                {
                    //AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
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


    // kills this gameobject..
    /**********************************************************************
        Enemy States
    **********************************************************************/
    private void Update_IdleState()
    {
        if (player == null || gruntStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        if(GetDistanceFromPlayer() <= gruntStats.chasingRadius && HasLineOfSightToPlayer(gameObject))
        {
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    private void Update_ChasingState()
    {
        animator.SetFloat("Range", GetDistanceFromPlayer());
        if (IsOnNavMeshOfflink())
        {
            gruntState = GruntState.PreJump;
            animator.PlayAnimation("Grunt Jump");
            NavigationAPI.stopAgent(gameObject);
            LookAt(GetTargetJumpPosition());
            return;
        }
        if (GetDistanceFromPlayer() > gruntStats.chasingRadius || !HasLineOfSightToPlayer(gameObject))
        {
            animator.PlayAnimation("Grunt Idle (Base)");
            gruntState = GruntState.Idle;
            NavigationAPI.stopAgent(gameObject);
            return;
        }
        // Change State
        if (GetDistanceFromPlayer() <= gruntStats.attackRadius)
        {
            animator.PlayAnimation("Grunt Attack");
            gruntState = GruntState.Attacking;
            NavigationAPI.stopAgent(gameObject);
            return;
        }
        LookAt(player);
        // Move Enemy 
        MoveToNavMeshPosition(player.transform.position);
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
    private void Update_PreJump() { }
    private void Update_Jump()
    {
        if (IsJumpFinished())
        {
            gruntState = GruntState.Idle;
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
    /****************************************************************
        Animation Events
    ****************************************************************/
    public void Slash()
    {

        emitter.emit(1000);

        if (hitbox != null)
            Destroy(hitbox);
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
        //AudioAPI.PlaySound(gameObject, "enemyattack_sfx");
        if (hitboxPrefab == null)
            return;
        hitbox = Instantiate(hitboxPrefab);
        if(hitbox!= null && hitboxPosition!= null){
            hitbox.transform.localPosition = hitboxPosition.transform.position;
            EnemyHitBox enemyHitBox = hitbox.getScript<EnemyHitBox>();
            if (enemyHitBox != null && gruntStats != null)
                enemyHitBox.SetDamage(gruntStats.damage);
        }
    }
    public void EndDeath()
    {
       if(gameObject != null)
       Destroy(gameObject);
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
}