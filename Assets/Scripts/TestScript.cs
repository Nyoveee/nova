class TestScript : Script
{
    TestScript2? testScript2;
    protected override void init()
    {
        testScript2 = getScript<TestScript2>();
        Input.MapKey(Key.A, OnKeyPressA, OnKeyReleaseA);
        Input.MapKey(Key._0, OnKeyPressA, OnKeyReleaseA);
    }
    private void OnKeyPressA()
    {
        Console.WriteLine("Pressed A");
    }
    private void OnKeyReleaseA()
    {
        Console.WriteLine("Released A");
    }
    protected override void update()
    {
        if (testScript2 != null)
            testScript2.MoveTable();
    }  
}
