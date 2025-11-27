// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class ButtonScript : Script
{
    public Scene nextScene;

    private bool isDragging = false;
    private Vector2 mouseInitialPosition;

    private Vector3 initialPosition;

    // This function is first invoked when game starts.
    protected override void init()
    {
        initialPosition = gameObject.transform.position;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (isDragging) { 
            // calculate distance from initial click
            Vector2 difference = Input.GetUIMousePosition() - mouseInitialPosition;

            gameObject.transform.position = new Vector3(initialPosition.x + difference.x, initialPosition.y, 0f);
        }
    }

    public void onHover()
    {
        // AudioAPI.PlaySound(gameObject, "metal_pipe");
    }

    public void onPressed()
    {
        isDragging = true;
        mouseInitialPosition = Input.GetUIMousePosition();
    }

    public void onReleased()
    {
        isDragging = false;
        initialPosition = gameObject.transform.position;

        //Debug.Log("Released");
        //SceneAPI.ChangeScene(nextScene);
    }
}