// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnemyAnimationEvent : Script
{
    private Enemy enemyScript;
    [SerializableField]
    private string triggerMethod1;
    [SerializableField]
    private string triggerMethod2;
    [SerializableField]
    private string triggerMethod3;
    [SerializableField]
    private string triggerMethod4;
    [SerializableField]
    private string triggerMethod5;
    [SerializableField]
    private string triggerMethod6;
    [SerializableField]
    private string triggerMethod7;
    [SerializableField]
    private string triggerMethod8;
    protected override void init(){ enemyScript = gameObject.GetParent().getScript<Enemy>();}
    public void TriggerEvent1() { CallMethod(triggerMethod1); }
    public void TriggerEvent2() { CallMethod(triggerMethod2); }
    public void TriggerEvent3() { CallMethod(triggerMethod3); }
    public void TriggerEvent4() { CallMethod(triggerMethod4); }
    public void TriggerEvent5() { CallMethod(triggerMethod5); }
    public void TriggerEvent6() { CallMethod(triggerMethod6); }
    public void TriggerEvent7() { CallMethod(triggerMethod7); }
    public void TriggerEvent8() { CallMethod(triggerMethod8); }
    private void CallMethod(string method)
    {
        System.Reflection.MethodInfo? function = enemyScript.GetType().GetMethod(method);
        if (function != null)
            function.Invoke(enemyScript, null);
    }
}