// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MainSettingsScript : Script
{
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {}

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void toShowSettingsUI(bool toShow)
    {
        if (toShow) 
        {
            gameObject.SetActive(true);

            foreach (GameObject buttonGameObject in GameObject.FindGameObjectsWithTag("ButtonToBeDisabledBySettings"))
            {
                Button_ button = buttonGameObject.getComponent<Button_>();

                if (button != null)
                {
                    button.isInteractable = false;
                }
            }
        }
        else
        {
            gameObject.SetActive(false);

            foreach(GameObject buttonGameObject in GameObject.FindGameObjectsWithTag("ButtonToBeDisabledBySettings"))
            {
                Button_ button = buttonGameObject.getComponent<Button_>();
                
                if(button != null)
                {
                    button.isInteractable = true;
                }
            }
        }
    }
}