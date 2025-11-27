// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PlayerCameraMove : Script
{
    [SerializableField]
    private Transform_? playerCameraPos = null; //Camera rotation is handled by PlayerRotateController which in unaffected by inheritence 

    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {
        //Invoke(MoveToOrientation, 0);


    }

    private void MoveToOrientation()
    {
        gameObject.transform.position = playerCameraPos.position;
    }

}