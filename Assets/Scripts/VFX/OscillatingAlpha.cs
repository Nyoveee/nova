// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class OscillatingAlpha : Script
{
    //[SerializableField] private float ratio = 1f;
    [SerializableField] private float speedMultiplier = 1f;

    private float timeElapsed = 0f;
    private ColorAlpha initialColor;

    private Image_ image;
    
    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        image = getComponent<Image_>();
        initialColor = image.colorTint;
    }

    // This function is invoked every update.
    protected override void update()
    {
        timeElapsed += Time.V_DeltaTime();
        timeElapsed = timeElapsed % (Mathf.Deg2Rad * 360f);

        float interval = Mathf.Sin(timeElapsed * speedMultiplier) / 2f + 0.5f;
        image.colorTint = new ColorAlpha(initialColor.r, initialColor.g, initialColor.b, interval);

        //gameObject.transform.localScale = initialScale + interval * ratio * initialScale;
    }

}