class TestScript : Script
{
    public Transform_? transform = null;
    public TestScript2? testScript2 = null;
    [SerializableField]
    protected int integer = 0;
    [SerializableField]
    public Vector2 test = new Vector2(0, 0);

    protected override void init()
    {
        transform = getComponent<Transform_>();
        testScript2 = getScript<TestScript2>();
        Input.MapKey(Key.A, OnKeyPressA, OnKeyReleaseA);
        Input.MapKey(Key._0, OnKeyPressA, OnKeyReleaseA);
    }
    private void OnKeyPressA()
    {
        //Console.WriteLine("Pressed A");
        Console.WriteLine(Input.mousePosition);
    }
    private void OnKeyReleaseA()
    {
        test = new Vector2(0, 0);
        if(transform != null)
            Debug.Print(test);

    }
    protected override void update()
    {
        if (testScript2 != null)
            testScript2.MoveTable();
    }  
}
