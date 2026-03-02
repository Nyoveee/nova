// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class RaiseEnemBoat : Script
{
    [SerializableField]
    private GameObject furthestDistance;

    [SerializableField] 
    private GameObject waveManager;
    private bool waveStarted = false;

    private bool isRising = false;
    private float riseTimer = 0f;
    private float bobTimer = 0f;

    private float riseSpeed = 2f;
    private float riseHeight = 70f;
    private float bobAmplitude = 5f;   // How much it shakes up/down
    private float bobFrequency = 1.5f;

    private float startY;
    private float targetY;
    private bool reachedTarget = false;

    private bool isSinking = false;
    private float sinkSpeed = 10f;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        startY = this.gameObject.transform.position.y;
        targetY = startY + riseHeight;
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(furthestDistance.getComponent<Transform_>().position.z < -600 && isSinking == false)
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
            }

            this.gameObject.transform.position = pos;
        }


        if (reachedTarget && !waveStarted)
        {
            waveManager.getScript<CannonWaveManager>().StartWave();
            waveStarted = true;
        }

        if (isSinking)
        {
            Vector3 pos = this.gameObject.transform.position;
            pos.y -= sinkSpeed * Time.V_DeltaTime();
            this.gameObject.transform.position = pos;

            if (pos.y <= startY)
            {
                pos.y = startY;
                this.gameObject.transform.position = pos;
                isSinking = false;
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
}