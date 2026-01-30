// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SliderScript : Script
{
    // Slider boundaries
    public float minX = 599f;   // left boundary
    public float maxX = 1299f;  // right boundary

    // Slider value settings
    public float minValue = 0f;
    public float maxValue = 100f;
    public float currentValue = 50f;

    public string fillEntityName = "Slider fill";
    public string saveKey = "SliderVolume"; // Must have to save/load the value
    public float maxVerticalDistance = 50f; // when u go to far it stops the mouse 

    private bool isDragging = false;
    private Vector2 mouseInitialPosition;
    private float clickOffset;
    private float sliderCenterY;

    protected override void init()
    {
        sliderCenterY = gameObject.transform.position.y;

        // load saved value
        LoadSliderValue();

        // Update initial position based on loaded value
        UpdateKnobPosition();
        UpdateFillBar();

        Debug.Log("Initial position X: " + gameObject.transform.position.x);
        Debug.Log("Loaded volume: " + currentValue);
    }
    protected override void update()
    {
        if (isDragging)
        {
            // get current mouse position
            Vector2 currentMousePos = Input.GetUIMousePosition();

            // check if mouse is too far away vertically
            float verticalDistance = currentMousePos.y - sliderCenterY;
            if (verticalDistance < 0) verticalDistance = -verticalDistance;

            if (verticalDistance > maxVerticalDistance)
            {
                //mouse is too far away, stop dragging
                isDragging = false;
                Debug.Log("Stopped dragging mouse too far away");
                return;
            }

            float newX = currentMousePos.x - clickOffset;

            // clamp the x position
            if (newX < minX) newX = minX;
            if (newX > maxX) newX = maxX;

            // update position
            Vector3 currentPos = gameObject.transform.position;
            gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);

            // calculate the slider value
            float sliderWidth = maxX - minX;
            float valuePercentage = (newX - minX) / sliderWidth;
            currentValue = minValue + (maxValue - minValue) * valuePercentage;

            //update the fill bar
            UpdateFillBar();
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

        // save the value when user releases the slider
        SaveSliderValue();

        Debug.Log("Final Value: " + currentValue);
        Debug.Log("Volume saved!");
    }

    // load the saved slider value
    private void LoadSliderValue()
    {
        try
        {
            float savedValue = PlayerPrefs.GetFloat(saveKey);
            currentValue = savedValue;
            Debug.Log("Loaded saved value: " + savedValue);
        }
        catch
        {
            // no saved value exists, use default
            currentValue = 30f;
            Debug.Log("No saved value found, using default: " + currentValue);
        }
    }

    // save the current slider value
    private void SaveSliderValue()
    {
        PlayerPrefs.SetFloat(saveKey, currentValue);
        PlayerPrefs.Save(); // Make sure it's saved in the json it created.
        Debug.Log("Saved value: " + currentValue);
    }

    // update knob position based on current value
    private void UpdateKnobPosition()
    {
        float sliderWidth = maxX - minX;
        float valuePercentage = (currentValue - minValue) / (maxValue - minValue);
        float newX = minX + (sliderWidth * valuePercentage);

        Vector3 currentPos = gameObject.transform.position;
        gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);
    }

    // update the fill bar based on current value
    private void UpdateFillBar()
    {
        GameObject fillObject = GameObject.Find(fillEntityName);
        if (fillObject != null)
        {
            float valuePercentage = (currentValue - minValue) / (maxValue - minValue);
            Vector3 fillScale = fillObject.transform.localScale;
            fillObject.transform.localScale = new Vector3(valuePercentage, fillScale.y, fillScale.z);
        }
    }
}