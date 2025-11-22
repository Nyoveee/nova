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
        Death
    }
    // State machine
    private GruntState gruntState = GruntState.Spawning;
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
        gameObject.transform.rotation = Quartenion.Identity();
        gruntStats = getScript<GruntStats>();

        animator.PlayAnimation("Grunt Idle (Base)");

        // Populate state machine dispatcher..
        updateState.Add(GruntState.Spawning, Update_SpawningState);
        updateState.Add(GruntState.Idle, Update_IdleState);
        updateState.Add(GruntState.Chasing, Update_ChasingState);
        updateState.Add(GruntState.Attacking, Update_AttackState);
        updateState.Add(GruntState.Death, Update_Death);

        rigidbody.SetVelocity(new Vector3(0, 0, 0));
        LookAtPlayer();

        Invoke(() =>
        {
            gruntState = GruntState.Idle;
        }, spawningDuration);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {   
        updateState[gruntState]();
    }
    /**********************************************************************
        Inheritted Functions
    **********************************************************************/
    public override bool IsEngagedInBattle()
    {
        return gruntState != GruntState.Idle;
    }

    public override void TakeDamage(float damage)
    {
        // blud already died let him die in peace dont take anymore damage..
        if(gruntState == GruntState.Death)
        {
            return;
        }

        AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        gruntStats.health -= damage;
        renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
        renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
        Invoke(() =>
        {
            renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
            renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
        }, hurtDuration);
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
         Destroy(gameObject);
    }
    /**********************************************************************
        Enemy States
    **********************************************************************/
    private void Update_SpawningState(){ }

    private void Update_IdleState()
    {
        if (player == null || gruntStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        if(GetDistanceFromPlayer() <= gruntStats.chasingRadius)
        {
            animator.PlayAnimation("Grunt Running");
            gruntState = GruntState.Chasing;
        }
    }
    private void Update_ChasingState()
    {
        animator.SetFloat("Range", GetDistanceFromPlayer());
        if (GetDistanceFromPlayer() > gruntStats.chasingRadius)
        {
            animator.PlayAnimation("Grunt Idle (Base)");
            gruntState = GruntState.Idle;
            rigidbody.SetVelocity(new Vector3(0, rigidbody.GetVelocity().y, 0));
            return;
        }
        // Change State
        if (GetDistanceFromPlayer() <= gruntStats.attackRadius)
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
        if (player == null || gruntStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        LookAtPlayer();
    }

    private void Update_Death(){}
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
        AudioAPI.PlaySound(gameObject, "enemyattack_sfx");
        if (hitboxPrefab == null)
            return;
        hitbox = Instantiate(hitboxPrefab);
        if(hitbox!= null && hitboxPosition!= null){
            hitbox.transform.position = hitboxPosition.transform.position;
            EnemyHitBox enemyHitBox = hitbox.getScript<EnemyHitBox>();
            if (enemyHitBox != null && gruntStats != null)
                enemyHitBox.SetDamage(gruntStats.damage);
        }
    }
}