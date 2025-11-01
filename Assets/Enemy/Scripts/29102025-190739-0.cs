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
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private EnemyStats? enemyStats = null;
    [SerializableField]
    private Animator_? animator = null;
    // This function is first invoked when game starts.
    protected override void init()
    {
        updateState.Add(EnemyState.Chasing, Update_ChasingState);
        updateState.Add(EnemyState.Attacking, Update_AttackState);
        player = GameObject.FindWithTag("Player");
        if(animator!= null && enemyStats != null)
        {
            animator.SetFloat("Health", enemyStats.health);
        }
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (animator != null && player != null)
        {
            distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
            animator.SetFloat("Range", distance);
        }
        updateState[enemyState]();
    }
    private void Update_ChasingState()
    {
        if(player == null || enemyStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        // Change State
        if(distance <= enemyStats.attackRadius)
        {
            enemyState = EnemyState.Attacking;
            // Change Animation
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
        if (player == null || enemyStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        LookAtPlayer();
        // Change State
        if (distance > enemyStats.attackRadius)
        {
            enemyState = EnemyState.Chasing;
        }

    }
    private void LookAtPlayer()
    {
        gameObject.transform.LookAt(player.transform);
        Vector3 eulerAngles = gameObject.transform.eulerAngles;
        eulerAngles.x = eulerAngles.z = 0;
        gameObject.transform.eulerAngles = eulerAngles;
    }
}