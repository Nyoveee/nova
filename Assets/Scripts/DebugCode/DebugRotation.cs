// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class DebugRotation : Script
{
    public Transform_ player;
    public Rigidbody_ rb;


    private float flight = 5f;


    // This function is first invoked when game starts.
    protected override void init()
    {
        rb.SetGravityFactor(0f);
        rb.SetVelocity(gameObject.transform.front * flight);

    }

    // This function is invoked every update.
    protected override void update()
    {


        //Quaternion targetRotation = Quaternion.LookRotation(directionVector);
        //    gameObject.transform.rotation = gameObject.transform.LookAt(player);
        //// gameObject.transform.rotation = targetRotation;


       // gameObject.transform.position = gameObject.transform.position + gameObject.transform.front * flight * Time.V_DeltaTime();




    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {
        //Vector3 directionVector = player.position - gameObject.transform.position;

        //directionVector.Normalize();
        //Quaternion targetRotation = Quaternion.LookRotation(directionVector);

        //rb.SetBodyRotation(targetRotation);


        //if (rb != null)
        //{
        //    rb.SetVelocity(gameObject.transform.front * flight);
        //}


    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}