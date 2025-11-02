// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class Enemy : Script
{
    private delegate void CurrentState();
    private enum EnemyState
    {
        Idle,
        Chasing,
        Attacking
    }
    private EnemyState enemyState = EnemyState.Idle;
    private Dictionary<EnemyState, CurrentState> updateState = new Dictionary<EnemyState, CurrentState>();
    private GameObject? player = null;
    private float distance = 0f;
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private EnemyStats? enemyStats = null;
    [SerializableField]
    private Animator_? animator = null;
    [SerializableField]
    private ParticleEmitter_ emitter = null;
    [SerializableField]
    private Rigidbody_? rigidbody = null;

    private Transform_? transform = null;
    // This function is first invoked when game starts.
    protected override void init()
    {
        if (animator != null)
            animator.PlayAnimation("Enemy Idle (Base)");
        updateState.Add(EnemyState.Idle, Update_IdleState);
        updateState.Add(EnemyState.Chasing, Update_ChasingState);
        updateState.Add(EnemyState.Attacking, Update_AttackState);
        player = GameObject.FindWithTag("Player");
        rigidbody.SetVelocity(new Vector3( 0, 0, 0));

        transform = getComponent<Transform_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        Vector3 playerPosition = new Vector3(player.transform.position.x, 0, player.transform.position.z);
        Vector3 enemyPosition = new Vector3(gameObject.transform.position.x,0,gameObject.transform.position.z);
        distance = Vector3.Distance(playerPosition, enemyPosition);
        updateState[enemyState]();
    }
    public bool IsEngagedInBattle()
    {
        return enemyState != EnemyState.Idle;
    }
    /**********************************************************************
        Enemy States
    **********************************************************************/
    private void Update_IdleState()
    {
        if (player == null || enemyStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        if(distance <= enemyStats.chasingRadius)
        {
            animator.PlayAnimation("Enemy Running");
            enemyState = EnemyState.Chasing;
        }
    }
    private void Update_ChasingState()
    {
        
        if(player == null || enemyStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        animator.SetFloat("Range", distance);
        if (distance > enemyStats.chasingRadius)
        {
            animator.PlayAnimation("Enemy Idle (Base)");
            enemyState = EnemyState.Idle;
            rigidbody.SetVelocity(new Vector3(0, rigidbody.GetVelocity().y, 0));
            return;
        }
        // Change State
        if (distance <= enemyStats.attackRadius)
        {
            if (animator != null)
                animator.PlayAnimation("Enemy Attack");
            rigidbody.SetVelocity(Vector3.Zero());
            enemyState = EnemyState.Attacking;
            return;
        }
        LookAtPlayer();
        // Move Enemy 
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        rigidbody.SetVelocity(direction * enemyStats.movementSpeed + new Vector3( 0,rigidbody.GetVelocity().y,0));

    }
    private void Update_AttackState()
    {
        if (player == null || enemyStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        LookAtPlayer();
    }
    private void LookAtPlayer()
    {
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
        AudioAPI.PlaySound(gameObject, "enemyattack_sfx");
        emitter.emit(1000);
    }
    public void EndAttack()
    {
        if (distance > enemyStats.attackRadius)
        {
            animator.PlayAnimation("Enemy Running");
            enemyState = EnemyState.Chasing;
        }
    }
}