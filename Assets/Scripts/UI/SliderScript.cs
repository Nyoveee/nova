// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SliderScript : Script
{
    // Slider properties
    public float minValue = 0f;
    public float maxValue = 100f;
    public float currentValue = 50f;

    // Width of the slider bar
    public float sliderWidth = 400f;
    // This function is first invoked when game starts.
    protected override void init()
    {
        Debug.Log("SliderScript initialized!");
        Debug.Log("Initial position: " + gameObject.transform.localPosition.x);
        //UpdateHandlePosition();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {

    }

    public void onPressed()
    {
        Debug.Log("Button pressed!");
        MoveRight();
    }

    public void onReleased()
    {
        Debug.Log("Button released!");
    }

    public void MoveRight()
    {
        Vector3 pos = gameObject.transform.localPosition;
        pos.x = pos.x + 1.0f;
        gameObject.transform.localPosition = pos;
        Debug.Log("Moved to: " + pos.x);
    }

}