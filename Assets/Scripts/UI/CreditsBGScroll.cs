// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class CreditsBGScroll : Script
{
    [SerializableField]
    private float startY = -3220f;

    [SerializableField]
    private float endY = 4294.322f;

    [SerializableField]
    private float scrollSpeed = 100f;

    [SerializableField]
    private float transitionDelay = 3f;

    [SerializableField]
    private Scene mainMenuScene;

    private bool isDone = false;
    private float delayTimer = 0f;

    protected override void awake()
    {}

    protected override void init()
    {}

    protected override void update()
    {
        if (isDone)
        {
            delayTimer += Time.V_DeltaTime();
            if (delayTimer >= transitionDelay)
                SceneAPI.ChangeScene(mainMenuScene);
            return;
        }

        Vector3 pos = gameObject.transform.position;
        pos.y += scrollSpeed * Time.V_DeltaTime();

        if (pos.y >= endY)
        {
            pos.y = endY;
            isDone = true;
        }

        gameObject.transform.position = pos;
    }

    protected override void fixedUpdate()
    {}

    protected override void exit()
    {}
}
