// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Switch : Script
{
    [SerializableField]
    private float switchActivationDistance = 10f;

    [SerializableField]
    private float rotation = 45f;

    [SerializableField]
    private float turningDuration = 0.4f;

    [SerializableField]
    private float turningLerpPower = 0.5f;

    [SerializableField]
    private MeshRenderer_ switchMesh;

    [SerializableField]
    private Audio switchSfx;

    private GameObject player;
    private bool isActivated = false;

    private bool isAnimating = false;
    private float timeElapsed = 0f;

    private bool isInteractable = false;

    private Quaternion initialRotation;
    private Quaternion finalRotation;

    private AudioComponent_ audioComponent;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        player = GameObject.FindWithTag("Player");
        initialRotation = gameObject.transform.localRotation;
        finalRotation = Quaternion.AngleAxis(Mathf.Deg2Rad * rotation, Vector3.Front()) * gameObject.transform.localRotation;

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

        Debug.Log(distance);

        isInteractable = distance <= switchActivationDistance;

        switchMesh?.setMaterialBool(1, "isActive", isInteractable);
    }

    private void animate()
    {

        timeElapsed += Time.V_DeltaTime();
        timeElapsed = Mathf.Min(turningDuration, timeElapsed);

        float interval = timeElapsed / turningDuration;

        gameObject.transform.localRotation = Quaternion.Slerp(initialRotation, finalRotation, Mathf.Pow(Mathf.SmoothLerp(0f, 1f, interval), turningLerpPower));

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
        if(!isActivated && isInteractable) {
            audioComponent?.PlaySound(switchSfx);

            isActivated = true;
            isAnimating = true;
            switchMesh?.setMaterialBool(1, "isActive", false);
        }
    }
}