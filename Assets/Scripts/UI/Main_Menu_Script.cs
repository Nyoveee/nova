// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Main_Menu_Script : Script
{
    [SerializableField] private Image_ pointer;
    [SerializableField] private float yOffset = 15f;

    private List<Button_> mainMenuButtons = new ();

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        foreach(GameObject child in gameObject.GetChildren())
        {
            Button_ button = child.getComponent<Button_>();

            if(button != null)
            {
                mainMenuButtons.Add(button);
            }
        }
    }   

    // This function is invoked every update.
    protected override void update()
    {
        // im just gonna poll lol :)

        foreach (Button_ button in mainMenuButtons)
        {
            // find the first hovered / pressed button..
            if (button.getState() != ButtonState.Hovered && button.getState() != ButtonState.Pressed)
            {
                continue;
            }

            pointer.gameObject.SetActive(true);

            Transform_ pointerTransform = pointer.gameObject.transform;
            Vector3 pointerPosition = pointerTransform.position;
            
            pointerTransform.position = new Vector3(pointerPosition.x, button.gameObject.transform.position.y + yOffset, pointerPosition.z);
            return;
        }

        // no buttons found..
        pointer.gameObject.SetActive(false);
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}