// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Charger : Script
{
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? stats = null;
    private Animator_? animator = null;
    private GameObject? player = null;
    private float distance = 0f;
    // State machine
    private enum ChargerState
    {
        Idle,
        Chasing,
        Charging,
        Stagger,
        Death
    }
    private ChargerState enemyState = ChargerState.Idle;
    private Dictionary<ChargerState, CurrentState> updateState = new Dictionary<ChargerState, CurrentState>();
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    protected override void init()
    {
        stats = getScript<ChargerStats>();
        animator = getComponent<Animator_>();
        player = GameObject.FindWithTag("Player");
        updateState.Add(ChargerState.Idle, Update_Idle);
        updateState.Add(ChargerState.Chasing, Update_Chasing);
        updateState.Add(ChargerState.Charging, Update_Charging);
        updateState.Add(ChargerState.Stagger, Update_Stagger);
        updateState.Add(ChargerState.Death, Update_Death);
    }
    protected override void update() {
        if (player != null) { 
            Vector3 playerPosition = new Vector3(player.transform.position.x, 0, player.transform.position.z);
            Vector3 chargerPosition = new Vector3(gameObject.transform.position.x, 0, gameObject.transform.position.z);
            distance = Vector3.Distance(playerPosition, chargerPosition);
        }
        updateState[enemyState](); 
    }
    /***********************************************************
        State
    ***********************************************************/
    private void Update_Idle(){}
    private void Update_Chasing() { }
    private void Update_Charging() { }
    private void Update_Stagger() { }
    private void Update_Death() { }
    /***********************************************************
        Animation Events
    ***********************************************************/
    public void StartCharging()
    {

    }
    public void EndCharge()
    {

    }
    public void StartStagger()
    {

    }
    public void EndStagger()
    {

    }
    /***********************************************************
        Collision Events
    ***********************************************************/
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player")
        {

        }
    }
}