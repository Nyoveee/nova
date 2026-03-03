// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ExpandingScale : Script
{
    [SerializableField] private float expandDuration = 2f;
    [SerializableField] private float initialScale = 0f;
    [SerializableField] private float finalScale = 100f;

    bool isExpanding = false;
    float timeElapsed = 0f;

    Vector3 initialScaleVector;
    Vector3 finalScaleVector;

    // This function is invoked once when gameobject is active.
    protected override void awake()
    {
        initialScaleVector = new Vector3(initialScale, initialScale, initialScale);
        finalScaleVector = new Vector3(finalScale, finalScale, finalScale);

        gameObject.transform.scale = initialScaleVector;
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!isExpanding) {
            return;
        }

        gameObject.transform.scale = Vector3.Lerp(initialScaleVector, finalScaleVector, timeElapsed / expandDuration);

        timeElapsed += Time.V_DeltaTime();

        if(timeElapsed > expandDuration)
        {
            isExpanding = false;
        }
    }

    public void expand()
    {
        timeElapsed = 0f;
        isExpanding = true;
    }
}