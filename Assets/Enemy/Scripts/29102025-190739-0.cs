// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class Enemy : Script
{
    delegate void CurrentState();
    private enum EnemyState
    {
        Chasing,
        Attacking
    }
    private EnemyState enemyState = EnemyState.Chasing;
    private Dictionary<EnemyState, CurrentState> updateState = new Dictionary<EnemyState, CurrentState>();
    private GameObject? player = null;
   
    private float distance = 0f;
    private float currentAttackTime = 0f;
    private bool emitted = false;
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
    // This function is first invoked when game starts.
    protected override void init()
    {
        if (animator != null)
            animator.PlayAnimation("Enemy Running");
        updateState.Add(EnemyState.Chasing, Update_ChasingState);
        updateState.Add(EnemyState.Attacking, Update_AttackState);
        player = GameObject.FindWithTag("Player");
        rigidbody.SetVelocity(new Vector3( 0, 0, 0));
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        updateState[enemyState]();
    }
    private void Update_ChasingState()
    {
        
        if(player == null || enemyStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        animator.SetFloat("Range", distance);
        // Change State
        if (distance <= enemyStats.attackRadius)
        {
            if (animator != null)
                animator.PlayAnimation("Enemy Attack");
            rigidbody.SetVelocity(Vector3.Zero);
            enemyState = EnemyState.Attacking;
            currentAttackTime = 0f;
            emitted = false;
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
        currentAttackTime += Time.V_FixedDeltaTime();
        if (player == null || enemyStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        LookAtPlayer();
        // Change State
        if(currentAttackTime >= enemyStats.particleEmitTime && !emitted)
        {
            emitted = true;
            emitter.emit(1000);
        }
        if (animator!= null && currentAttackTime >= enemyStats.attackTime)
        {
            if (distance <= enemyStats.attackRadius)
            {
                currentAttackTime = 0f;
                emitted = false;
            }
            else
            {
                animator.PlayAnimation("Enemy Running");
                enemyState = EnemyState.Chasing;
            }
              
        }

    }
    private void LookAtPlayer()
    {
        if(player!= null)
            gameObject.transform.LookAt(player.transform);
        Vector3 eulerAngles = gameObject.transform.eulerAngles;
        eulerAngles.x = eulerAngles.z = 0;
        gameObject.transform.eulerAngles = eulerAngles;
    }
}