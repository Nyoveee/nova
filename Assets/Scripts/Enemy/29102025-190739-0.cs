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
    [SerializableField]
    private EnemyStats? enemyStats = null;
    // This function is first invoked when game starts.
    protected override void init()
    {
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
        if(player == null || enemyStats == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        // Change State
        float distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        if(distance <= enemyStats.attackRadius)
        {
            enemyState = EnemyState.Attacking;
            // Change Animation
            return;
        }
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
        // Change State
        float distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        if (distance > enemyStats.attackRadius)
        {
            enemyState = EnemyState.Chasing;
        }

    }
}