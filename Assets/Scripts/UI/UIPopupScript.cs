// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class UIPopupScript : Script
{
    private delegate void Callback();

    [SerializableField]
    private float popupUIFadeDuration = 1f;

    private float timeElapsed = 0f;
    private bool isFading = false;

    private float initialAlpha = 0f;
    private float finalAlpha = 1f;

    private Canvas_ canvas;
    private Callback callback;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        canvas = getComponent<Canvas_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(!isFading)
        {
            return;
        }

        timeElapsed += Time.V_DeltaTime();

        canvas.alpha = Mathf.Interpolate(initialAlpha, finalAlpha, timeElapsed / popupUIFadeDuration, 1f);

        if (timeElapsed > popupUIFadeDuration) {
            isFading = false;

            if (finalAlpha == 0f)
            {
                // fade out..
                callback();
            }
            else
            {
                canvas.isInteractable = true;
                canvas.alpha = 1f;
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void toShowUI(bool toShow, Canvas_ parentCanvas)
    {
        if (isFading)
        {
            return;
        }

        timeElapsed = 0f;
        isFading = true;

        if (toShow) 
        {
            gameObject.SetActive(true);
            canvas = getComponent<Canvas_>();
            parentCanvas.isInteractable = false;

            canvas.alpha = 0f;
            initialAlpha = 0f;
            finalAlpha = 1f;

        }
        else
        {
            callback = () =>
            {
                gameObject.SetActive(false);
                parentCanvas.isInteractable = true;
            };

            canvas.isInteractable = false;
            canvas.alpha = 1f;
            initialAlpha = 1f;
            finalAlpha = 0f;
        }
    }
}