// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Runtime.CompilerServices;

class Charger : Script
{
    /***********************************************************
        Local Variables
    ***********************************************************/
    private delegate void CurrentState();
    private ChargerStats? chargerstats = null;
    private Animator_? animator = null;
    private Rigidbody_? rigidbody = null;
    private GameObject? player = null;
    private float distance = 0f;
    private float currentChargeTime = 0f;
    private float currentChargeCooldown = 0f;
    // State machine
    private enum ChargerState
    {
        Idle,
        Walk,
        Charging,
        Stagger,
        Death
    }
    private ChargerState chargerState = ChargerState.Idle;
    private Dictionary<ChargerState, CurrentState> updateState = new Dictionary<ChargerState, CurrentState>();
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    protected override void init()
    {
        chargerstats = getScript<ChargerStats>();
        animator = getComponent<Animator_>();
        rigidbody = getComponent<Rigidbody_>();
        player = GameObject.FindWithTag("Player");
        updateState.Add(ChargerState.Idle, Update_Idle);
        updateState.Add(ChargerState.Walk, Update_Walk);
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
        updateState[chargerState](); 
    }
    /***********************************************************
        Public Functions
    ***********************************************************/

    /***********************************************************
        Helper Functions
    ***********************************************************/
    private void LookAtPlayer()
    {
        if (player == null || gameObject.transform == null)
        {
            Debug.LogWarning("Missing Reference Found");
            return;
        }
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    /***********************************************************
        State
    ***********************************************************/
    private void Update_Idle(){
        if(distance < chargerstats.chasingRange)
        {
            chargerState = ChargerState.Walk;
            animator.PlayAnimation("Charger_Walk");
            return;
        }
    }
    private void Update_Walk() {
        if(distance > chargerstats.chasingRange)
        {
            chargerState = ChargerState.Idle;
            animator.PlayAnimation("Charger_Idle");
            rigidbody.SetVelocity(Vector3.Zero());
            return;
        }
        LookAtPlayer();
        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        currentChargeCooldown -= Time.V_FixedDeltaTime();
        if (distance < chargerstats.chargingRange && currentChargeCooldown <=0 )
        {
            chargerState = ChargerState.Charging;
            animator.PlayAnimation("Charger_Charge");
            currentChargeTime = chargerstats.maxChargeTime;
            rigidbody.SetVelocity(direction * chargerstats.movementSpeed * chargerstats.speedMultiplier + new Vector3(0, rigidbody.GetVelocity().y, 0));
            return;
        }
        // Move Enemy 
        rigidbody.SetVelocity(direction * chargerstats.movementSpeed + new Vector3(0, rigidbody.GetVelocity().y, 0));
    }
    private void Update_Charging() {
        currentChargeTime -= Time.V_FixedDeltaTime();
        if (currentChargeTime <= 0)
        {
            chargerState = ChargerState.Stagger;
            animator.PlayAnimation("Charger_Stagger");
            rigidbody.SetVelocity(Vector3.Zero());
        }
    }
    private void Update_Stagger() { }
    private void Update_Death() { }
    /***********************************************************
        Animation Events
    ***********************************************************/
    public void EndStagger()
    {
        chargerState = ChargerState.Walk;
        animator.PlayAnimation("Charger_Walk");
        currentChargeCooldown = chargerstats.chargeCooldown;
    }
    /***********************************************************
        Collision Events
    ***********************************************************/
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player" && chargerState == ChargerState.Charging)
        {
            chargerState = ChargerState.Stagger;
            animator.PlayAnimation("Charger_Stagger");
            rigidbody.SetVelocity(Vector3.Zero());
        }
    }
}