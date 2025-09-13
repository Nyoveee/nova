class TestScript : Script
{
    Transform_? transform;
    TestScript2? testScript2;
    protected override void init()
    {
        transform = getComponent<Transform_>();
        testScript2 = getScript<TestScript2>();
        Input.MapKey(Key.A, OnKeyPressA, OnKeyReleaseA);
        Input.MapKey(Key._0, OnKeyPressA, OnKeyReleaseA);
     
    }
    private void OnKeyPressA()
    {
        Console.WriteLine("Pressed A");
        //Console.WriteLine(Input.mousePosition);
    }
    private void OnKeyReleaseA()
    {
        Vector2 test;
        test.x = 0;
        test.y = 0;
        if(transform != null)
            Debug.Print(test);

    }
    protected override void update()
    {
        if (testScript2 != null)
            testScript2.MoveTable();
    }  
}
