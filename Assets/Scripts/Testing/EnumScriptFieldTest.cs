// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnumScriptFieldTest : Script
{
    public enum Test
    {
        Hi,
        Bye,
        NaisuGeimu
    }
    [SerializableField]
    private Test test;
    [SerializableField]
    private List<Test> test2;
    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        if (test == Test.NaisuGeimu)
            Debug.Log("Enum is NaisuGeimu");
        else
            Debug.Log("Enum is not NaisuGeimu");
    }


}