// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Shotgun : Gun
{
    // Inspector variables
    public required Transform_ camera;
    public required float cooldown = 1f;
    public required float recoilDuration = 0.2f;

    public required float range  = 30f;
    public required float scatterRange = 0.5f;
    public required float damage = 20f;
    public required int numOfPellets = 5;

    // Runtime variables
    private float timeElapsed = 0f;
    private bool onCooldown = false;
    private bool isRecoiling = false;
    private Vector3 startPosition;
    private Vector3 backPosition;

    private System.Random random = new System.Random();

    protected override void update()
    {
        if (!onCooldown)
        {
            return;
        }

        // Handle gun recoil lerping..
        if (timeElapsed < recoilDuration)
        {
            gameObject.transform.localPosition = Vector3.Lerp(startPosition, backPosition, Mathf.Pow(timeElapsed / recoilDuration, 0.3f));
        }
        else if (timeElapsed < recoilDuration * 2)
        {
            gameObject.transform.localPosition = Vector3.Lerp(backPosition, startPosition, Mathf.Pow((timeElapsed - recoilDuration) / recoilDuration, (1 / 0.3f)));
        }
        else if (isRecoiling)
        {
            isRecoiling = false;
            gameObject.transform.localPosition = startPosition;
        }

        if (timeElapsed > cooldown)
        {
            timeElapsed = 0f;
            onCooldown = false;
        }
        else
        {
            timeElapsed += Time.V_FixedDeltaTime();
        }
    }

    // Returns true if the gun is off cooldown and a fire happens.
    public override bool Fire()
    {
        if (onCooldown)
        {
            return false;
        }

        onCooldown = true;
        isRecoiling = true;

        AudioAPI.PlaySound(gameObject, "Shotgun Fire SFX");

        for(int i = 0; i < numOfPellets; ++i)
        {
            Vector3 offset = new Vector3(
                (float) (random.NextDouble() - 0.5) * scatterRange,
                (float) (random.NextDouble() - 0.5) * scatterRange,
                (float) (random.NextDouble() - 0.5) * scatterRange
            );

            Vector3 randomFront = camera.front + offset;
            randomFront.Normalize();

            if(RayCastFire(camera.position, randomFront, range, damage))
            {
                // Debug.Log(string.Format("Pellet {0} hit!", i));
            }
        }

        startPosition = gameObject.transform.localPosition;
        backPosition = startPosition - (Vector3.Front() * 2f);

        return true;
    }
}