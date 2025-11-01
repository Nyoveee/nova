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
    // This function is first invoked when game starts.
    protected override void init()
    {
        if (animator != null)
            animator.PlayAnimation("Enemy Running");
        updateState.Add(EnemyState.Chasing, Update_ChasingState);
        updateState.Add(EnemyState.Attacking, Update_AttackState);
        player = GameObject.FindWithTag("Player");
    }

    // This function is invoked every fixed update.
    protected override void update()
    {      
        updateState[enemyState]();
    }
    private void Update_ChasingState()
    {
        
        if(player == null || enemyStats == null || animator == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        animator.SetFloat("Range", distance);
        // Change State
        if (distance <= enemyStats.attackRadius)
        {
            if (animator != null)
                animator.PlayAnimation("Enemy Attack");
            enemyState = EnemyState.Attacking;
            currentAttackTime = 0f;
            emitted = false;
            return;
        }
        LookAtPlayer();
        // Move Enemy using transform for now
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        gameObject.transform.position = gameObject.transform.position + direction * enemyStats.movementSpeed*Time.V_FixedDeltaTime();

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
        if (currentAttackTime >= enemyStats.attackTime)
        {
            if (animator != null)
                animator.PlayAnimation("Enemy Running");
            enemyState = EnemyState.Chasing;
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