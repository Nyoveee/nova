class TestScript : Script
{
    TestScript2? testScript2;
    protected override void init()
    {
        testScript2 = getComponent<TestScript2>();
    }

    protected override void update()
    {
        if (testScript2 != null)
            testScript2.MoveTable();
    }  
}
