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

    private bool isInteractable = false;

    private Quaternion initialRotation;
    private Quaternion finalRotation;

    private AudioComponent_ audioComponent;

    private bool isActivated = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        isActivated = !isEnabledAtStart;

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

        if(isActivated)
        {
            return;
        }

        float distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        isInteractable = distance <= switchActivationDistance;

        switchMesh?.setMaterialBool(1, "isActive", isInteractable);
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

    private void handleSwitchActivation()
    {
        float distance = Vector3.Distance(player.transform.position, gameObject.transform.position);
        isInteractable = distance <= switchActivationDistance;

        if (!isActivated && isInteractable) {
            audioComponent?.PlaySound(switchSfx);

            isActivated = true;
            isAnimating = true;
            switchMesh?.setMaterialBool(1, "isActive", false);
        }
    }

    public bool isSwitchActivated()
    {
        return isActivated;
    }

    public void activateSwitch()
    {
        // isActivated here means has it been used.
        isActivated = true;
    }
    public void deactivateSwitch()
    {
        isActivated = false;
        isAnimating = false;
        timeElapsed = 0f;
        switchMesh?.setMaterialBool(1, "isActive", true);
        switchMesh.gameObject.transform.rotation = initialRotation;
    }
}