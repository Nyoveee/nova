// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.CompilerServices;
public abstract class Enemy : Script
{
    [SerializableField]
    protected float hurtDuration = 0.1f;
    [SerializableField]
    private Prefab ichorPrefab;
    [SerializableField]
    private GameObject ichorSpawnPoint;
    /***********************************************************
        Local Variables
    ***********************************************************/
    protected GameObject? player = null;
    protected Animator_? animator = null;
    protected Rigidbody_? rigidbody = null;
    protected SkinnedMeshRenderer_? renderer = null;
    private EnemyStats? enemyStats = null;
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
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        Vector3 direction = player.transform.position - gameObject.transform.position;
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
        GameObject ichor = Instantiate(ichorPrefab);    
        ichor.transform.position = ichorSpawnPoint.transform.position;
    }
    /***********************************************************
        Script Functions
    ***********************************************************/
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        animator = getComponent<Animator_>();
        renderer = getComponent<SkinnedMeshRenderer_>();
        enemyStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
    }
   
}