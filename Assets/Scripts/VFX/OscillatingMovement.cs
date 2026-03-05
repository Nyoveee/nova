// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class OscillatingMovement : Script
{
    [SerializableField] private Vector3 offset = new Vector3( 0f, 0f, 0f );
    [SerializableField] private float speedMultiplier = 1f;

    private float timeElapsed = 0f;
    private Vector3 initialPosition;
    
    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        initialPosition = gameObject.transform.localPosition;
    }

    // This function is invoked every update.
    protected override void update()
    {
        timeElapsed += Time.V_DeltaTime();
        timeElapsed = timeElapsed % (Mathf.Deg2Rad * 360f);

        float interval = Mathf.Sin(timeElapsed);

        gameObject.transform.localPosition = initialPosition + interval * offset;
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}