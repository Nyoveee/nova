// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.CompilerServices;
public abstract class Enemy : Script
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/

    [SerializableField]
    private Prefab ichorPrefab;
    [SerializableField]
    private GameObject ichorSpawnPoint;
    [SerializableField]
    protected Animator_? animator = null;
    [SerializableField]
    protected SkinnedMeshRenderer_? renderer = null;
    [SerializableField]
    protected NavMeshAgent_? navMeshAgent = null;
    /***********************************************************
        Local Variables
    ***********************************************************/
    protected GameObject? player = null;
    private EnemyStats? enemyStats = null;
    private bool wasRecentlyDamaged = false;
    private float ichorSpawnPositionVariance = 1.5f;
    /***********************************************************
        Enemy Types must inherited from this
    ***********************************************************/
    public abstract void TakeDamage(float damage);
    public abstract bool IsEngagedInBattle();
    /***********************************************************
        Shared Functions
    ***********************************************************/
    protected void LookAtPlayer()
    {
        if (player == null)
            return;
        Vector3 direction = player.transform.position - renderer.gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    protected void LookAtObject(GameObject @object)
    {
        if(@object == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        Vector3 direction = @object.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    protected float GetDistanceFromPlayer()
    {
        return player != null ? Vector3.Distance(player.transform.position, gameObject.transform.position) : 0f;
    }
    protected void SpawnIchor()
    {
        for (int i = 0; i < enemyStats.ichorSpawnAmount; ++i){
            Vector3 direction = new Vector3(0, Random.Range(-1f,1f), 0);
            direction.Normalize();
            float spawnDistance = Random.Range(0, ichorSpawnPositionVariance);
            GameObject ichor = Instantiate(ichorPrefab, ichorSpawnPoint.transform.position + direction * spawnDistance);
        }
    }
    protected void MoveToNavMeshPosition(Vector3 position)
    {
        RayCastResult? result = PhysicsAPI.Raycast(position, -Vector3.Up(), 1000f);
        if (result != null)
            NavigationAPI.setDestination(gameObject, result.Value.point);
    }
    protected bool WasRecentlyDamaged()
    {
        return wasRecentlyDamaged;
    }
    protected void TriggerRecentlyDamageCountdown()
    {
        wasRecentlyDamaged = true;
        Invoke(() =>
        {
            wasRecentlyDamaged = false;
        }, enemyStats.hurtDuration);
    }
    /***********************************************************
       Script Functions
    ***********************************************************/
    /***********************************************************
        Script Functions
    ***********************************************************/
    protected override void init()
    {
        enemyStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
    }
   
}