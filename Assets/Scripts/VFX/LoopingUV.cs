// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class LoopingUV : Script
{
    public enum Direction
    {
        Horizontal,
        Vertical
    };

    public Direction direction;

    public int materialIndex = 0;
    public float speedMultiplier = 1f;

    private MeshRenderer_ meshRenderer;
    private Vector2 uvOffset = new Vector2(0, 0);

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
    }

    // This function is invoked every update.
    protected override void update()
    {
        if(direction == Direction.Horizontal)
        {
            uvOffset.x += speedMultiplier * Time.V_DeltaTime();
            uvOffset.x %= 1f;
        }
        else
        {
            uvOffset.y -= speedMultiplier * Time.V_DeltaTime();
            uvOffset.y %= 1f;
        }

        meshRenderer.setMaterialVector2(materialIndex, "UVOffset", uvOffset);
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}