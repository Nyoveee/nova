// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class test2 : Script
{
    private Transform_ transform;
    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        transform.localPosition = new Vector3(transform.localPosition.x + 1 * Time.V_FixedDeltaTime(), transform.localPosition.y, transform.localPosition.z);
    }

}