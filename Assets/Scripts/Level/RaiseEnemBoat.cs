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
    [SerializableField] 
    private float scrollSpeed = 50f;
    [SerializableField] 
    private GameObject explosionEmitter;
    [SerializableField]
    private GameObject smallExplosionEmitter;
    [SerializableField]
    private GameObject scatterExplosionEmitter;
    [SerializableField]
    private GameObject smoke1;
    [SerializableField]
    private GameObject smoke2;

    private bool isRising = false;
    private float riseTimer = 0f;
    private float bobTimer = 0f;

    private float riseSpeed = 2f;
    private float riseHeight = 55f;
    private float bobAmplitude = 3f;   // How much it shakes up/down
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
    private Quaternion startRotation;
    private bool rotationCaptured = false;

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

                explosionEmitter.getComponent<ParticleEmitter_>().emit();
                smallExplosionEmitter.getComponent<ParticleEmitter_>().emit();
                scatterExplosionEmitter.getComponent<ParticleEmitter_>().emit();

                smoke1.SetActive(true);
                smoke2.SetActive(true);

                CameraAPI.shakeCamera(1f, 1f);

                Sink();
            }
        }

        if (isSinking)
        {
            if (!rotationCaptured)
            {
                startRotation = gameObject.transform.rotation;
                rotationCaptured = true;
            }

            Vector3 pos = this.gameObject.transform.position;
            float newY = pos.y - sinkSpeed * Time.V_DeltaTime();
            pos.z -= scrollSpeed * Time.V_DeltaTime();
            if (newY <= startY + 20f)
            {
                newY = startY + 20f;
                isSinking = false;
                hasSunk = true;
            }
            pos.y = newY;

            this.gameObject.transform.position = pos;

            Quaternion tiltTarget = startRotation
                                    * Quaternion.AngleAxis(8f, new Vector3(0f, 0.707f, -0.707f));
            gameObject.transform.rotation = Quaternion.RotateTowards(
                gameObject.transform.rotation,
                tiltTarget,
                2f * Time.V_DeltaTime()
            );
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