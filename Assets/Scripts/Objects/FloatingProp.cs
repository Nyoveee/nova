// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class FloatingProp : Script
{
    [SerializableField]
    private float floatingDistance = 2.5f;
    [SerializableField]
    private float floatingSpeed = 2f;

    private float currentFloatingTime;
    private Vector3 floatingPosition;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        currentFloatingTime += Random.Range(0, Mathf.Deg2Rad * 360f);
        floatingPosition = gameObject.transform.localPosition;
    }

    // This function is invoked every update.
    protected override void update()
    {
        currentFloatingTime += Time.V_DeltaTime() * floatingSpeed;
        float yOffset = Mathf.Sin(currentFloatingTime) * floatingDistance;
        gameObject.transform.localPosition = floatingPosition + new Vector3(0, yOffset, 0);
    }

}