// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SlashVFX : Script
{
    public float delay = 0.3f;
    public float initialPower = 20f;
    public float finalPower = 5f;

    public float powerLerpDuration = 1f;
    public float rotationDuration = 1f;

    public float rotationAngle = 90f;

    MeshRenderer_ meshRenderer;
    
    float rotationTimeElapsed = 0f;
    float powerTimeElapsed = 0f;

    Quaternion initialRotation;
    Quaternion finalRotation;

    bool isAnimating = false;
    bool hasShakenCamera = false;

    public float SmoothStep(float t)
    {
        return t * t * (3.0f - 2.0f * t);
    }

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
        initialRotation = gameObject.transform.rotation;

        gameObject.transform.rotate(gameObject.transform.up, -rotationAngle);
        finalRotation = gameObject.transform.rotation;

        gameObject.transform.rotation = initialRotation;

        Invoke(() =>
        {
            isAnimating = true;
        }, delay);

        meshRenderer.setMaterialFloat(0, "alpha", 0);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!isAnimating)
        {
            return;
        }

        AnimateTransparency();
        AnimateRotation();
    }

    private void AnimateTransparency()
    {
        if (powerTimeElapsed < powerLerpDuration / 2f)
        {
            float interval = powerTimeElapsed / (powerLerpDuration / 2f);
            meshRenderer.setMaterialFloat(0, "power", Mathf.Interpolate(initialPower, finalPower, interval, 1f));
            meshRenderer.setMaterialFloat(0, "alpha", interval);
        }
        else if (powerTimeElapsed < powerLerpDuration)
        {
            if(!hasShakenCamera)
            {
                hasShakenCamera = true;
                CameraAPI.shakeCamera(0.2f, 3f);
            }

            float interval = (powerTimeElapsed - powerLerpDuration / 2f) / (powerLerpDuration / 2f);
            meshRenderer.setMaterialFloat(0, "power", Mathf.Interpolate(finalPower, initialPower, interval, 1f));
            meshRenderer.setMaterialFloat(0, "alpha", 1f - interval);
        }
        else
        {
            isAnimating = false;
        }

        powerTimeElapsed += Time.V_DeltaTime();
    }

    private void AnimateRotation()
    {
        if (rotationTimeElapsed > rotationDuration)
        {
            return;
        }

        gameObject.transform.localRotation = Quaternion.Slerp(initialRotation, finalRotation, Mathf.Pow(SmoothStep(rotationTimeElapsed / rotationDuration), 3f));
        rotationTimeElapsed += Time.V_DeltaTime();
    }
}