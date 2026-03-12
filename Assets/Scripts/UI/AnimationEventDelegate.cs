// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class AnimationEventDelegate : Script
{
    [SerializableField]
    private BossIntroCutscene bossIntroCutscene;

    [SerializableField]
    private string functionName;

    [SerializableField]
    private string functionName2;

    public void TriggerEvent() { CallMethod(functionName); }
    public void TriggerEvent2() { CallMethod(functionName2); }

    private void CallMethod(string method)
    {
        System.Reflection.MethodInfo? function = bossIntroCutscene.GetType().GetMethod(method);

        if (function != null)
            function.Invoke(bossIntroCutscene, null);
    }

}