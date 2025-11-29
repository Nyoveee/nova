// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class TurnoffObject : Script
{




    // This function is first invoked when game starts.
    protected override void init()
    {
        MapKey(Key.MouseRight, DisableMe);


    }

    void DisableMe()
    {
        this.gameObject.transform.position = this.gameObject.transform.position + gameObject.transform.front * 5f;
        gameObject.SetActive(false);

    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}