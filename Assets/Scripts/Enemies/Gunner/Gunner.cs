// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Gunner : Enemy
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float hurtDuration = 0.1f;
    [SerializableField]
    private Material defaultMaterial;
    [SerializableField]
    private Material hurtMaterial;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private enum GunnerState
    {
        Idle,
        Walk,
        Shoot,
        Stagger,
        Death
    }
    private GunnerState gunnerState = GunnerState.Idle;
    private Dictionary<GunnerState, CurrentState> updateState = new Dictionary<GunnerState, CurrentState>();
    private float currentHurtTime = 0f;
    /***********************************************************
        Components
    ***********************************************************/
    private GunnerStats? gunnerStats = null;
    private GameGlobalReferenceManager gameGlobalReferenceManager = null;
    /**********************************************************************
        Script Functions
    **********************************************************************/
    protected override void init()
    {
        base.init();
        gunnerStats = getScript<GunnerStats>();
        updateState.Add(GunnerState.Idle, Update_Idle);
        updateState.Add(GunnerState.Walk, Update_Walk);
        updateState.Add(GunnerState.Shoot, Update_Shoot);
        updateState.Add(GunnerState.Stagger, Update_Stagger);
        updateState.Add(GunnerState.Death, Update_Death);
        gameGlobalReferenceManager = GameObject.FindWithTag("Game Global Reference Manager").getScript<GameGlobalReferenceManager>();
        Debug.Log("Total Vantage Points:" + gameGlobalReferenceManager.vantagePoints.Length.ToString());
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (currentHurtTime > 0)
        {
            currentHurtTime -= Time.V_FixedDeltaTime();
            if (currentHurtTime <= 0)
                renderer.changeMaterial(0, defaultMaterial);
        }
        updateState[gunnerState]();
    }
    /***********************************************************
       Inherited Functions
    ***********************************************************/
    public override void TakeDamage(float damage)
    {
        if (gunnerState == GunnerState.Death)
            return;
        gunnerStats.health -= damage;
        AudioAPI.PlaySound(gameObject, "Enemy Hurt SFX");
        renderer.changeMaterial(0, hurtMaterial);
        currentHurtTime = hurtDuration;
        if (gunnerStats.health <= 0)
        {
            gunnerState = GunnerState.Death;
            animator.PlayAnimation("Gunner_Die");
            return;
        }
        gunnerState = GunnerState.Stagger;
        animator.PlayAnimation("Gunner_Stagger");

    }
    public override bool IsEngagedInBattle()
    {
        return gunnerState != GunnerState.Idle && gunnerState != GunnerState.Death;
    }
    /**********************************************************************
       Enemy States
    **********************************************************************/
    private void Update_Idle()
    {

    }
    private void Update_Walk()
    {

    }
    private void Update_Shoot()
    {

    }
    private void Update_Stagger()
    {

    }
    private void Update_Death(){}
    /**********************************************************************
       Animation Events
    **********************************************************************/
    public void EndStagger()
    {
        gunnerState = GunnerState.Idle;
        animator.PlayAnimation("Gunner_Idle");
    }
    public void EndDeath()
    {
        ObjectAPI.Destroy(gameObject);
    }
}