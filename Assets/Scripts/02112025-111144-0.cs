// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class TEST : Script
{
    public Transform_? transform;

    private bool startMoving = false;

    // This function is first invoked when game starts.
    protected override void init()
    {
        // transform = getComponent<Transform_>();
        // Input.MapKey(Key.Space, toggle);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        //if (startMoving) {
        //    transform.position = new Vector3 ( transform.position.x + Time.V_DeltaTime() * 5, transform.position.y, transform.position.z );
        //}
    }

    private void toggle()
    {
        //startMoving = !startMoving; 
    }

    protected override void onCollisionEnter(GameObject other)
    {
        Debug.Log("collision");
    }

}