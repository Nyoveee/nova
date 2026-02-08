// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SliderScript : Script
{
    // Sliders Components
    [SerializableField]
    private GameObject sliderFill;
    // Slider boundaries
    private float knobMinX;  
    private float knobMaxX;  

    // Slider value settings
    public float minValue;
    public float maxValue;
    public float defaultValue;

    // Must have to save/load the value
    public string saveKey;

    private float maxVerticalDistance = 50f; // when u go to far it stops the mouse 
    private bool isDragging = false;
    private Vector2 mouseInitialPosition;
    private float clickOffset;
    private float sliderCenterY;

    protected override void init()
    {
        sliderCenterY = gameObject.transform.position.y;
        knobMinX = sliderFill.transform.position.x;
        knobMaxX = sliderFill.transform.position.x + sliderFill.transform.scale.x;
        // load saved value
        LoadSliderValue();

        // Update initial position based on loaded value
        UpdateKnobPosition();
        UpdateFillBar();

        Debug.Log("Initial position X: " + gameObject.transform.position.x);
        Debug.Log("Loaded volume: " + defaultValue);
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
                SaveSliderValue();
                Debug.Log("Stopped dragging mouse too far away");
                return;
            }

            float newX = currentMousePos.x - clickOffset;

            // clamp the x position
            if (newX < knobMinX) newX = knobMinX;
            if (newX > knobMaxX) newX = knobMaxX;

            // update position
            Vector3 currentPos = gameObject.transform.position;
            gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);

            // calculate the slider value
            float sliderWidth = knobMaxX - knobMinX;
            float valuePercentage = (newX - knobMinX) / sliderWidth;
            defaultValue = minValue + (maxValue - minValue) * valuePercentage;

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

        Debug.Log("Final Value: " + defaultValue);
        Debug.Log("Volume saved!");
    }

    // load the saved slider value
    private void LoadSliderValue()
    {
        try
        {
            float savedValue = PlayerPrefs.GetFloat(saveKey);
            defaultValue = savedValue;
            Debug.Log("Loaded saved value: " + savedValue);
        }
        catch
        {
            // no saved value exists, use default
            defaultValue = 30f;
            Debug.Log("No saved value found, using default: " + defaultValue);
        }
    }

    // save the current slider value
    private void SaveSliderValue()
    {
        PlayerPrefs.SetFloat(saveKey, defaultValue);
        PlayerPrefs.Save(); // Make sure it's saved in the json it created.
        Debug.Log("Saved value: " + defaultValue);
    }

    // update knob position based on current value
    private void UpdateKnobPosition()
    {
        float sliderWidth = knobMaxX - knobMinX;
        float valuePercentage = (defaultValue - minValue) / (maxValue - minValue);
        float newX = knobMinX + (sliderWidth * valuePercentage);

        Vector3 currentPos = gameObject.transform.position;
        gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);
    }

    // update the fill bar based on current value
    private void UpdateFillBar()
    {
        if (sliderFill != null)
        {
            float valuePercentage = (defaultValue - minValue) / (maxValue - minValue);
            Vector3 fillScale = sliderFill.transform.localScale;
            sliderFill.transform.localScale = new Vector3(valuePercentage, fillScale.y, fillScale.z);
        }
    }
}