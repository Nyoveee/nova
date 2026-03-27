// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Net;

class GoddessBehaviour : Script
{
    enum GoddessState
    {
        Idle,
        Rising,
        Floating,
        Disappear,
    }
    [SerializableField]
    private float floatingDistance;
    [SerializableField]
    private float floatingSpeed;
    [SerializableField]
    private float risingTime;
    [SerializableField]
    private float risingSlowDownRate;
    [SerializableField]
    private float delayForVoiceOver;
    [SerializableField]
    private float rotationTime;
    [SerializableField]
    private float disappearTime;

    private Animator_ animator;
    private SkinnedMeshRenderer_ skinnedMeshRenderer_;

    // Rising State
    private float currentOscillation;
    private float currentRisingTime;
    private Vector3 risingStartPoint;
    private Vector3 risingEndPoint;

    // Floating
    private Vector3 floatingPosition;
    private float currentFloatingTime;

    // Voiceline
    private VoiceoverScript voiceoverScript;

    private List<string> goddessVoiceOverText;
    private List<float> goddessVoiceOverTime;
    private Audio goddessVoiceOverAudio;

    // State
    private delegate void CurrentState();
    private GoddessState goddessState = GoddessState.Idle;
    private Dictionary<GoddessState, CurrentState> updateState = new Dictionary<GoddessState, CurrentState>();

    // Pointing
    private Quaternion startRotation;
    private Quaternion endRotation;
    private GameObject pointedGameObject;
    private float timeToStartPointing;
    private bool b_IsRotatingToPoint;
    private float currentRotationTime;

    // Disappear
    private float currentDisappearTime;
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
        skinnedMeshRenderer_ = getComponent<SkinnedMeshRenderer_>();
        animator = getComponent<Animator_>();
        updateState.Add(GoddessState.Idle, Update_Idle);
        updateState.Add(GoddessState.Rising, Update_Rising);
        updateState.Add(GoddessState.Floating, Update_Floating);
        updateState.Add(GoddessState.Disappear, Update_Disappear);
    }
    protected override void update()
    {
        // Goddess can point at any state
        if (b_IsRotatingToPoint)
        {
            currentRotationTime += Time.V_DeltaTime();
            currentRotationTime = Mathf.Min(currentRotationTime, rotationTime);
            gameObject.transform.rotation = Quaternion.Slerp(startRotation, endRotation, currentRotationTime / rotationTime);
            if (currentRotationTime == rotationTime)
            {
                b_IsRotatingToPoint = false;
                animator.PlayAnimation("Goddess Point");
            }
        }
        updateState[goddessState]();
    }
    private void Update_Idle(){}
    private void Update_Rising()
    {
        // Rising
        currentRisingTime += Time.V_DeltaTime();
        currentRisingTime = Mathf.Min(currentRisingTime, risingTime);
        gameObject.transform.position = Vector3.Interpolate(risingStartPoint, risingEndPoint, currentRisingTime / risingTime, risingSlowDownRate);
        // Alpha
        for (int i = 0; i < 6; ++i)
            skinnedMeshRenderer_.setMaterialFloat(i, "alpha", currentRisingTime / risingTime);
        // Change to float
        if (currentRisingTime == risingTime)
        {
            BeginFloat(gameObject.transform.position);
            return;
        }
    }

    private void Update_Floating()
    {
        currentFloatingTime += Time.V_DeltaTime() * floatingSpeed;
        float yOffset = Mathf.Sin(currentFloatingTime) * floatingDistance;
        gameObject.transform.position = floatingPosition + new Vector3(0, yOffset, 0);
    }
    
    private void Update_Disappear()
    {
        currentDisappearTime -= Time.V_DeltaTime();
        if (currentDisappearTime <= 0)
            Destroy(gameObject);
        for (int i = 0; i < 6; ++i)
            skinnedMeshRenderer_.setMaterialFloat(i, "alpha", currentDisappearTime / disappearTime);

    }
    public void SetFloatingSpeech(List<string> voiceOverText, List<float> voiceOverTime, Audio voiceOverAudio)
    {
        goddessVoiceOverText = voiceOverText;
        goddessVoiceOverTime = voiceOverTime;
        goddessVoiceOverAudio = voiceOverAudio;

    }
    public void BeginRising(Vector3 startPoint, Vector3 endPoint)
    {
        currentOscillation = 0;
        goddessState = GoddessState.Rising;
        risingStartPoint = startPoint;
        risingEndPoint = endPoint;
    }
    public void BeginFloat(Vector3 position)
    {
        floatingPosition = position;
        currentFloatingTime = 0;
        goddessState = GoddessState.Floating;
        if (goddessVoiceOverText.Count > 0)
        {
            Invoke(() =>
            {
                voiceoverScript.TriggerVoiceOverSequence("Goddess", goddessVoiceOverText, goddessVoiceOverAudio, goddessVoiceOverTime, false);
            }, delayForVoiceOver);
           
        } 
    }
    public void BeginDisappearing()
    {
        goddessState = GoddessState.Disappear;
        currentDisappearTime = disappearTime;
    }
    public void Idle()
    {
        goddessState = GoddessState.Idle;
    }
    // Begin Pointing at this gameobject, at this time
    public void PointAt(GameObject go, float timeToStartPointing)
    {
        pointedGameObject = go;
        startRotation = gameObject.transform.rotation;
        Vector3 direction = go.transform.position - gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();
        endRotation = Quaternion.LookRotation(direction) * Quaternion.AngleAxis(180 * Mathf.Deg2Rad, new Vector3(0,1,0));
        this.timeToStartPointing = timeToStartPointing;
        if (pointedGameObject != null)
            Invoke(() => { b_IsRotatingToPoint = true; }, timeToStartPointing);
    }
    public bool IsDisappearing() => (goddessState == GoddessState.Disappear);
    /***********************************************************
        Animation Events
    ***********************************************************/

    public void StopPointing()
    {
        animator.PlayAnimation("Goddess Idle");
        pointedGameObject = null;
    }
   
}