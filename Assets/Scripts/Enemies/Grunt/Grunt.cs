// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class Grunt : Script
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
    private float hurtDuration = 0.1f;

    [SerializableField]
    private Material? defaultMaterial = null;

    [SerializableField]
    private Material? hurtMaterial = null;

    /***********************************************************
        Components
    ***********************************************************/
    private GruntStats? gruntStats = null;
    private Animator_? animator = null;
    private Rigidbody_? rigidbody = null;
    private Transform_? transform = null;
    private SkinnedMeshRenderer_? renderer = null;
    /***********************************************************
        Runtime variables..
    ***********************************************************/
    private enum GruntState
    {
        Idle,
        Chasing,
        Attacking,
        Death
    }

    // State machine
    private GruntState gruntState = GruntState.Idle;
    private Dictionary<GruntState, CurrentState> updateState = new Dictionary<GruntState, CurrentState>();

    // Player Reference
    private GameObject? player = null;
    private float distanceToPlayer = 0f;

    // Hurt
    private float hurtTimeElapsed = 0f;
    private bool recentlyTookDamage = false;

    // Attack
    private GameObject? hitbox = null;


    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
        rigidbody = getComponent<Rigidbody_>();
        animator = getComponent<Animator_>();
        renderer = getComponent<SkinnedMeshRenderer_>();
        gruntStats = getScript<GruntStats>();
        if (animator != null)
            animator.PlayAnimation("Grunt Idle (Base)");

        // Populate state machine dispatcher..
        updateState.Add(GruntState.Idle, Update_IdleState);
        updateState.Add(GruntState.Chasing, Update_ChasingState);
        updateState.Add(GruntState.Attacking, Update_AttackState);
        updateState.Add(GruntState.Death, Update_Death);

        player = GameObject.FindWithTag("Player");

        rigidbody.SetVelocity(new Vector3(0, 0, 0));
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        // update take damage logic..
        if(recentlyTookDamage)
        {
            if (hurtTimeElapsed > hurtDuration)
            {
                recentlyTookDamage = false;
                renderer.changeMaterial(0, defaultMaterial);
            }
            else
            {
                hurtTimeElapsed += Time.V_FixedDeltaTime();
            }
        }

        Vector3 playerPosition = new Vector3(player.transform.position.x, 0, player.transform.position.z);
        Vector3 enemyPosition = new Vector3(gameObject.transform.position.x,0,gameObject.transform.position.z);

        distanceToPlayer = Vector3.Distance(playerPosition, enemyPosition);
        
        updateState[gruntState]();
    }
    /**********************************************************************
        Public Functions
    **********************************************************************/
    public bool IsEngagedInBattle()
    {
        return gruntState != GruntState.Idle;
    }

    public void TakeDamage(float damage)
    {
        // blud already died let him die in peace dont take anymore damage..
        if(recentlyTookDamage || gruntState == GruntState.Death)
        {
            return;
        }

        recentlyTookDamage = true;
        hurtTimeElapsed = 0f;

        AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        gruntStats.health -= damage;
        renderer.changeMaterial(0, hurtMaterial);

        if(gruntStats.health <= 0)
        {
            gruntState = GruntState.Death;
            animator.PlayAnimation("Grunt Death");
            rigidbody.SetVelocity(Vector3.Zero());
        }
    }

    // kills this gameobject..
    public void Die()
    {
        // ObjectAPI.Destroy(gameObject);
    }
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
        if(distanceToPlayer <= gruntStats.chasingRadius)
        {
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    private void Update_ChasingState()
    {
        
        if(player == null || gruntStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        animator.SetFloat("Range", distanceToPlayer);
        if (distanceToPlayer > gruntStats.chasingRadius)
        {
            animator.PlayAnimation("Grunt Idle (Base)");
            gruntState = GruntState.Idle;
            rigidbody.SetVelocity(new Vector3(0, rigidbody.GetVelocity().y, 0));
            return;
        }
        // Change State
        if (distanceToPlayer <= gruntStats.attackRadius)
        {
            if (animator != null)
                animator.PlayAnimation("Grunt Attack");
            rigidbody.SetVelocity(Vector3.Zero());
            gruntState = GruntState.Attacking;
            return;
        }
        LookAtPlayer();
        // Move Enemy 
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        rigidbody.SetVelocity(direction * gruntStats.movementSpeed + new Vector3( 0,rigidbody.GetVelocity().y,0));

    }
    private void Update_AttackState()
    {
        LookAtPlayer();
    }

    private void Update_Death()
    {

    }

    private void LookAtPlayer()
    {
        if (player == null || transform == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        transform.setFront(direction);
    }
    /****************************************************************
        Animation Events
    ****************************************************************/
    public void Slash()
    {

        emitter.emit(1000);
        if (hitbox != null)
            ObjectAPI.Destroy(hitbox);
    }
    public void EndAttack()
    {
        Debug.Log("end of attack....");

        if (distanceToPlayer > gruntStats.attackRadius)
        {
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    public void BeginSwing()
    {
        AudioAPI.PlaySound(gameObject, "enemyattack_sfx");
        if (hitboxPrefab == null)
            return;
        hitbox = ObjectAPI.Instantiate(hitboxPrefab);
        if(hitbox!= null && hitboxPosition!= null){
            hitbox.transform.position = hitboxPosition.transform.position;
            EnemyHitBox enemyHitBox = hitbox.getScript<EnemyHitBox>();
            if (enemyHitBox != null && gruntStats != null)
                enemyHitBox.SetDamage(gruntStats.damage);
        }
           
    }
}