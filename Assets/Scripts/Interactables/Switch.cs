// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Switch : Script
{
    [SerializableField]
    private float switchActivationDistance = 10f;

    [SerializableField]
    private float rotation = 110f;

    [SerializableField]
    private float turningDuration = 0.4f;

    [SerializableField]
    private float turningLerpPower = 0.5f;

    [SerializableField]
    private MeshRenderer_ switchMesh;

    [SerializableField]
    private Audio switchSfx;

    [SerializableField]
    private bool isEnabledAtStart = true;

    private GameObject player;

    private bool isAnimating = false;
    private float timeElapsed = 0f;


    private Quaternion initialRotation;
    private Quaternion finalRotation;

    private AudioComponent_ audioComponent;

    private bool isEnabled = false;
    private bool hasBeenActivated = false;
    
    private bool isPlayerCloseToSwitch = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        isEnabled = isEnabledAtStart;

        player = GameObject.FindWithTag("Player");
        initialRotation = gameObject.transform.rotation;
        finalRotation = Quaternion.AngleAxis(Mathf.Deg2Rad * rotation, gameObject.transform.front) * gameObject.transform.rotation;

        MapKey(Key.E, handleSwitchActivation);

        audioComponent = getComponent<AudioComponent_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(isAnimating)
        {
            animate();
        }

        if(hasBeenActivated)
        {
            return;
        }

        if(!isEnabled)
        {
            return;
        }
            
        // visual indicator..
        float distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        isPlayerCloseToSwitch = distance <= switchActivationDistance;

        switchMesh?.setMaterialBool(1, "isActive", isPlayerCloseToSwitch);
    }

    private void animate()
    {
        timeElapsed += Time.V_DeltaTime();
        timeElapsed = Mathf.Min(turningDuration, timeElapsed);

        float interval = timeElapsed / turningDuration;

        if (switchMesh != null)
        {
            switchMesh.gameObject.transform.rotation = Quaternion.Slerp(initialRotation, finalRotation, Mathf.Pow(Mathf.SmoothLerp(0f, 1f, interval), turningLerpPower));
        }

        if (timeElapsed == turningDuration)
        {
            isAnimating = false;
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void handleSwitchActivation()
    {
        if (!hasBeenActivated && isPlayerCloseToSwitch) {
            audioComponent?.PlaySound(switchSfx);

            hasBeenActivated = true;
            isAnimating = true;
            switchMesh?.setMaterialBool(1, "isActive", false);
        }
    }

    public bool isSwitchActivated()
    {
        return hasBeenActivated;
    }
    public void forceEnableSwitch()
    {
        hasBeenActivated = true;
        isAnimating = true;
        switchMesh?.setMaterialBool(1, "isActive", false);
    }
    public void enableSwitch()
    {
        isEnabled = true;
        deactivateSwitch();
    }

    public void deactivateSwitch()
    {
        hasBeenActivated = false;
        isAnimating = false;
        timeElapsed = 0f;
        switchMesh.gameObject.transform.rotation = initialRotation;
    }
}