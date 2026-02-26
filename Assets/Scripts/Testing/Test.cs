// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Test : Script
{
    [SerializableField] private Prefab shockwaveAttack;

    [SerializableField] private float minInterval = 1f;
    [SerializableField] private float maxInterval = 3f;

    [SerializableField] private float slamDuration = 0.2f;
    [SerializableField] private float slamRecoilDuration = 0.1f;
    [SerializableField] private float slamDistance = 200f;
    [SerializableField] private float recoilDistance = 32f;

    private float slamTimeElapsed = 0f;
    private float nextInterval;
    private float timeElapsed = 0f;
    private bool animatingSlam = false;
    private bool hasSlamed = false;

    private Vector3 initialPosition;
    private Vector3 slamPosition;
    private Vector3 slamRecoilPosition;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {
    }

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        initialPosition = gameObject.transform.position;
        slamPosition = new Vector3(initialPosition.x, initialPosition.y - slamDistance, initialPosition.z);
        slamRecoilPosition = new Vector3(slamPosition.x, slamPosition.y + recoilDistance, slamPosition.z);

        nextInterval = Random.Range(minInterval, maxInterval);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(animatingSlam)
        {
            slamTimeElapsed += Time.V_DeltaTime();

            if (slamTimeElapsed > slamDuration + slamRecoilDuration)
            {
                slamTimeElapsed = 0;
                animatingSlam = false;
            }
            else if (slamTimeElapsed > slamDuration) 
            {
                // animate recoil
                float interval = (slamTimeElapsed - slamDuration) / slamRecoilDuration;
                gameObject.transform.position = Vector3.Lerp(slamPosition, slamRecoilPosition, Mathf.Pow(interval, 1 / 2.3f));
                
                if(!hasSlamed)
                {
                    Instantiate(shockwaveAttack, slamPosition);
                    hasSlamed = true;
                }
            }
            else
            {
                // animate slam
                float interval = slamTimeElapsed / slamDuration;
                gameObject.transform.position = Vector3.Lerp(initialPosition, slamPosition, Mathf.Pow(interval, 2.3f));
            }
        }
        else
        {
            gameObject.transform.position = Vector3.Lerp(slamRecoilPosition, initialPosition, timeElapsed / nextInterval);
            timeElapsed += Time.V_DeltaTime();

            if (timeElapsed > nextInterval)
            {
                timeElapsed = 0;
                nextInterval = Random.Range(minInterval, maxInterval);

                // start coroutine. (sike no coroutine)
                animatingSlam = true;
                hasSlamed = false;
            }
        }

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}