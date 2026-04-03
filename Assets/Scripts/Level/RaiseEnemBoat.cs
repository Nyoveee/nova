// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class RaiseEnemBoat : Script
{
    [SerializableField]
    private Transform_ furthestDistance;

    [SerializableField]
    private float distance = -400f;
    
    [SerializableField] 
    private GameObject waveManager;
    private bool waveStarted = false;

    [SerializableField]
    private GameObject stalacite;
    [SerializableField]
    private float driftSpeed = 20f;
    [SerializableField]
    private float hitThreshold = 100f;
    [SerializableField]
    private float stalactiteXOffset = 30f;

    private bool isRising = false;
    private float riseTimer = 0f;
    private float bobTimer = 0f;

    private float riseSpeed = 2f;
    private float riseHeight = 50f;
    private float bobAmplitude = 5f;   // How much it shakes up/down
    private float bobFrequency = 1.5f;

    private float startY;
    private float targetY;
    private bool reachedTarget = false;

    private bool isSinking = false;
    private float sinkSpeed = 10f;
    private bool hasSunk = false;

    private float swayAmplitude = 20f;
    private float swayFrequency = 0.8f;
    private float startX;

    private bool isOutro = false;
    private bool isDrifting = false;
    private bool hitStalactite = false;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        startY = this.gameObject.transform.position.y;
        startX = this.gameObject.transform.position.x;

        targetY = startY + riseHeight;
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (furthestDistance.position.z < distance && isSinking == false && !hasSunk)
        {
            isRising = true;
        }

        if (isRising)
        {
            riseTimer += Time.V_DeltaTime();
            Vector3 pos = this.gameObject.transform.position;

            if (!reachedTarget)
            {
                float t = riseTimer * riseSpeed;
                if (t > 1f) t = 1f;
                pos.y = startY + (targetY - startY) * t;

                if (pos.y >= targetY - 0.05f)
                {
                    reachedTarget = true;
                    bobTimer = 0f;
                }
            }
            else
            {
                bobTimer += Time.V_DeltaTime();
                pos.y = targetY + Mathf.Sin(riseTimer * bobFrequency) * bobAmplitude;
                pos.x = startX + Mathf.Sin(riseTimer * swayFrequency) * swayAmplitude;
            }

            this.gameObject.transform.position = pos;
        }


        if (reachedTarget && !waveStarted)
        {
            waveManager.getScript<CannonWaveManager>().StartWave();
            waveStarted = true;
        }

        if (isDrifting && !hitStalactite)
        {
            riseTimer += Time.V_DeltaTime();

            Vector3 pos = this.gameObject.transform.position;
            float targetX = stalacite.transform.position.x + stalactiteXOffset;

            float step = driftSpeed * Time.V_DeltaTime();
            float diff = targetX - pos.x;
            if (Mathf.Abs(diff) <= step)
                pos.x = targetX;
            else if (diff > 0)
                pos.x += step;
            else
                pos.x -= step;

            pos.y = targetY + Mathf.Sin(riseTimer * bobFrequency) * bobAmplitude;
            this.gameObject.transform.position = pos;

            float zDist = Mathf.Abs(pos.z - stalacite.transform.position.z);
            if (zDist <= hitThreshold)
            {
                hitStalactite = true;
                Sink();
            }
        }

        if (isSinking)
        {
            Vector3 pos = this.gameObject.transform.position;
            pos.y -= sinkSpeed * Time.V_DeltaTime();
            pos.z = stalacite.transform.position.z;
            this.gameObject.transform.position = pos;

            Quaternion tiltTarget = Quaternion.AngleAxis(-40f, new Vector3(1f, 0f, 0f))
                                  * Quaternion.AngleAxis(25f, new Vector3(0f, 0f, 1f));
            gameObject.transform.rotation = Quaternion.RotateTowards(
                gameObject.transform.rotation,
                tiltTarget,
                40f * Time.V_DeltaTime()
            );

            if (pos.y <= startY)
            {
                pos.y = startY;
                this.gameObject.transform.position = pos;
                isSinking = false;
                hasSunk = true;
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void Sink()
    {
        isSinking = true;
        isRising = false;
        reachedTarget = false;
    }

    public void StartOutro()
    {
        isRising = false;
        reachedTarget = false;
        isOutro = true;
        isDrifting = true;
    }
}