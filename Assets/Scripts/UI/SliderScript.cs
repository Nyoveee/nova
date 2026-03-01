// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

enum Settings
{
    MasterVolume,
    BGMVolume,
    SFXVolume,
    MouseSensitivity,
    Gamma,
    Brightness
};

class SliderScript : Script
{
    // Sliders Components
    [SerializableField]
    private Image_ sliderFill;

    [SerializableField]
    private Settings settings;

    [SerializableField]
    private float maxValue = 1;

    [SerializableField]
    private float minValue = 0;

    // Slider boundaries
    private float knobMinX;  
    private float knobMaxX;  

    // hmm..
    private float maxVerticalDistance = 50f; // when u go to far it stops the mouse 

    private bool isDragging = false;
    private Vector2 mouseInitialPosition;
    private float clickOffset;
    private float sliderCenterY;

    private float currentValue;
    
    protected override void init()
    {
        sliderCenterY = gameObject.transform.position.y;
        knobMinX = sliderFill.gameObject.transform.position.x - sliderFill.gameObject.transform.scale.x / 2f;
        knobMaxX = sliderFill.gameObject.transform.position.x + sliderFill.gameObject.transform.scale.x / 2f;

        // load saved value
        LoadSliderValue();
        UpdateKnobPosition();
        UpdateFillBar();
    }

    protected override void update()
    {
        if (isDragging)
        {
            // get current mouse position
            Vector2 currentMousePos = Input.GetUIMousePosition();

            // check if mouse is too far away vertically
#if false
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
#endif
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
            currentValue = minValue + (maxValue - minValue) * valuePercentage;

            // update the fill bar
            UpdateFillBar();

            // save the value as it drags (i wanna hear the volume change live)
            SaveSliderValue();
        }
    }

    public void onPressed()
    {
        isDragging = true;

        mouseInitialPosition = Input.GetUIMousePosition();
        float currentHandleX = gameObject.transform.position.x;
        clickOffset = mouseInitialPosition.x - currentHandleX;
    }

    public void onReleased()
    {
        isDragging = false;
    }

    // load the saved slider value
    private void LoadSliderValue()
    {
        switch(settings)
        {
            case Settings.MasterVolume:
                currentValue = AudioAPI.GetMasterVolume();
                break;
            case Settings.BGMVolume:
                currentValue = AudioAPI.GetBGMVolume();
                break;
            case Settings.SFXVolume:
                currentValue = AudioAPI.GetSFXVolume();
                break;
            case Settings.Gamma:
                currentValue = RendererAPI.gamma;
                break;
            case Settings.Brightness:
                currentValue = 1f;
                break;
            case Settings.MouseSensitivity:
                currentValue = PlayerPrefs.GetFloat("MouseSensitivity", 1f);
                break;
        }
    }

    // save the current slider value
    private void SaveSliderValue()
    {
        switch (settings)
        {
            case Settings.MasterVolume:
                AudioAPI.SetMasterVolume(currentValue);
                break;
            case Settings.BGMVolume:
                AudioAPI.SetBGMVolume(currentValue);
                break;
            case Settings.SFXVolume:
                AudioAPI.SetSFXVolume(currentValue);
                break;
            case Settings.Gamma:
                RendererAPI.gamma = currentValue;
                break;
            case Settings.Brightness:
                currentValue = 1f;
                break;
            case Settings.MouseSensitivity:
                PlayerPrefs.SetFloat("MouseSensitivity", currentValue);

                // We update the current player in the scene if any..
                PlayerRotateController playerRotateController = GameObject.FindWithTag("PlayerHead")?.getScript<PlayerRotateController>();

                if (playerRotateController != null) {
                    playerRotateController.SetMouseSensitivity(currentValue);
                }

                break;
        }
    }

    // update knob position based on current value
    private void UpdateKnobPosition()
    {
        float sliderWidth = knobMaxX - knobMinX;
        float valuePercentage = (currentValue - minValue) / (maxValue - minValue);
        float newX = knobMinX + (sliderWidth * valuePercentage);

        Vector3 currentPos = gameObject.transform.position;
        gameObject.transform.position = new Vector3(newX, currentPos.y, 0f);
    }

    // update the fill bar based on current value
    private void UpdateFillBar()
    {
        if (sliderFill != null)
        {
            float valuePercentage = (currentValue - minValue) / (maxValue - minValue);
            sliderFill.textureCoordinatesRange = new Vector2(valuePercentage, 1.0f);
        }
    }
}