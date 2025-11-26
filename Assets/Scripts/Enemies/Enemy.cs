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
    // Jump
    private float currentJumpDuration = 0f;
    private NavMeshOfflinkData offlinkData;
    private float verticalMaxJumpHeight;
    /***********************************************************
        Enemy Types must inherited from this
    ***********************************************************/
    public abstract void TakeDamage(float damage);
    public abstract bool IsEngagedInBattle();
    /***********************************************************
       Public Functions
    ***********************************************************/
    public void Explode()
    {
        Destroy(gameObject);
        for (int i = 0; i < enemyStats.ichorExplodeSpawnAmount; ++i)
        {
            Vector3 direction = new Vector3(0, Random.Range(-1f, 1f), 0);
            direction.Normalize();
            float spawnDistance = Random.Range(0, ichorSpawnPositionVariance);
            GameObject ichor = Instantiate(ichorPrefab);
            ichor.transform.position = ichorSpawnPoint.transform.position + direction * spawnDistance;
        }
    }
    /***********************************************************
        Shared Functions
    ***********************************************************/
    protected Vector3 GetTargetJumpPosition()
    {
        return offlinkData.endNode;
    }
    protected bool IsCurrentlyJumping()
    {
        return offlinkData.valid && navMeshAgent.isOnOffMeshLinks();
    }
    protected bool IsJumpFinished()
    {
        float t = currentJumpDuration / enemyStats.jumpDuration;
        Vector3 horizontalPos = Vector3.Lerp(offlinkData.startNode, offlinkData.endNode, t);
        float yOffset = verticalMaxJumpHeight * 4f * (t - t * t);
        gameObject.transform.position = horizontalPos + Vector3.Up() * yOffset;
        currentJumpDuration += Time.V_FixedDeltaTime();
        return currentJumpDuration >= enemyStats.jumpDuration;
    }

    protected void LookAt(GameObject @object)
    {
        if (@object == null)
            return;
        Vector3 direction = @object.transform.position - renderer.gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    protected void LookAt(Vector3 position)
    {
        Vector3 direction = position - renderer.gameObject.transform.position;
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
            GameObject ichor = Instantiate(ichorPrefab);
            ichor.transform.position = ichorSpawnPoint.transform.position + direction * spawnDistance;
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
    protected bool IsOnNavMeshOfflink() {
        if (!navMeshAgent.isOnOffMeshLinks())
            return false;
        offlinkData = navMeshAgent.getOffLinkData();
        if (offlinkData.valid){
            verticalMaxJumpHeight = MathF.Abs(offlinkData.endNode.y - offlinkData.startNode.y);
            verticalMaxJumpHeight = 0.25f + 0.5f * verticalMaxJumpHeight; // 0.25f adds small curve
            currentJumpDuration = 0f;
        }
        
        return offlinkData.valid;
    }
    /***********************************************************
       Script Functions
    ***********************************************************/
    protected override void init()
    {
        enemyStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
        navMeshAgent.setAutomateNavMeshOfflinksState(false);
    }

}