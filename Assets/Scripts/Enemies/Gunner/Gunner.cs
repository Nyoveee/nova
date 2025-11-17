// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Gunner : Enemy
{
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
    /**********************************************************************
       Script Functions
   **********************************************************************/
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {}
    /***********************************************************
       Inherited Functions
   ***********************************************************/
    public override void TakeDamage(float damage)
    {
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

}