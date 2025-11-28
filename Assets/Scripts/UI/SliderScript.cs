// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SliderScript : Script
{
    // Slider boundaries
    public float minX = 599f;   // Left boundary
    public float maxX = 1299f;  // Right boundary

    // Slider value settings
    public float minValue = 0f;
    public float maxValue = 100f;
    public float currentValue = 50f;

    public string fillEntityName = "Slider fill"; 
    public float maxVerticalDistance = 50f; // when u go to far it stops the mouse 

    private bool isDragging = false;
    private Vector2 mouseInitialPosition;
    private float clickOffset;
    private float sliderCenterY; 

    // This function is first invoked when game starts.
    protected override void init()
    {
        sliderCenterY = gameObject.transform.position.y;
        Debug.Log("Initial position X: " + gameObject.transform.position.x);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (isDragging)
        {
            // Get current mouse position
            Vector2 currentMousePos = Input.GetUIMousePosition();

            // Check if mouse is too far away vertically
            float verticalDistance = currentMousePos.y - sliderCenterY;
            if (verticalDistance < 0) verticalDistance = -verticalDistance; 

            if (verticalDistance > maxVerticalDistance)
            {
                //mouse is too far away, stop dragging
                isDragging = false;
                Debug.Log("Stopped dragging - mouse too far away");
                return;
            }

            float newX = currentMousePos.x - clickOffset;

            // clamp the x position
            if (newX < minX) newX = minX;
            if (newX > maxX) newX = maxX;

            // Update position
            Vector3 currentPos = gameObject.transform.position;
            gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);

            // calculate the slider value
            float sliderWidth = maxX - minX;
            float valuePercentage = (newX - minX) / sliderWidth;
            currentValue = minValue + (maxValue - minValue) * valuePercentage;

            // Update the fill bar
            GameObject fillObject = GameObject.Find(fillEntityName);
            if (fillObject != null)
            {
                Vector3 fillScale = fillObject.transform.localScale;
                fillObject.transform.localScale = new Vector3(valuePercentage, fillScale.y, fillScale.z);
            }
        }
    }
    public void onPressed()
    {
        isDragging = true;
        mouseInitialPosition = Input.GetUIMousePosition();
        float currentHandleX = gameObject.transform.position.x;
        clickOffset = mouseInitialPosition.x - currentHandleX;

        Debug.Log("Click offset: " + clickOffset);
    }

    public void onReleased()
    {
        isDragging = false;
        Debug.Log("Final Value: " + currentValue);
    }

}