// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GunAnimationDelegate : Script
{
    [SerializableField]
    private PlayerWeaponController playerWeaponController;

    [SerializableField]
    private string functionName;

    [SerializableField]
    private string functionName2;

    [SerializableField]
    private string functionName3;

    [SerializableField]
    private string functionName4;

    public void TriggerEvent() { CallMethod(functionName); }
    public void TriggerEvent2() { CallMethod(functionName2); }

    public void TriggerEvent3() { CallMethod(functionName3); }
    public void TriggerEvent4() { CallMethod(functionName4); }

    private void CallMethod(string method)
    {
        System.Reflection.MethodInfo? function = playerWeaponController.GetType().GetMethod(method);

        if (function != null)
            function.Invoke(playerWeaponController, null);
    }
}