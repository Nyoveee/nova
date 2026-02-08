// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PulsatingLight : Script
{
    public float minIntensity = 1f;
    public float maxIntensity = 2f;
    public float minRadius = 4f;
    public float maxRadius = 6f;
    public float speedMultiplier = 1f;

    private Light_ light;
    private float timeElapsed = 0f;

    public bool isActive = true;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        light = getComponent<Light_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(!isActive)
        {
            return;
        }

        if(light == null)
        {
            return;
        }

        timeElapsed += Time.V_DeltaTime() * speedMultiplier;

        // loop..
        timeElapsed %= 360 * Mathf.Deg2Rad;

        float sine = Mathf.Sin(timeElapsed);

        // remap range..
        sine = (sine + 1) / 2;

        light.intensity = Mathf.Interpolate(minIntensity, maxIntensity, sine, 1f);
        light.radius = Mathf.Interpolate(minRadius, maxRadius, sine, 1f);
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}