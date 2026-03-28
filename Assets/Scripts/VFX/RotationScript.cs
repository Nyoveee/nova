// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class RotationScript : Script
{
    public Vector3 axis = Vector3.Up();
    public float rotationSpeed = 1.0f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {
        gameObject.transform.localRotate(axis, rotationSpeed);
    }
}