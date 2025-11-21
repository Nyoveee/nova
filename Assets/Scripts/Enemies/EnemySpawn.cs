// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnemySpawn : Script
{
    Animator_? animator;
    
    void StartSpawn()
    {
        animator = getComponent<Animator_>();
        foreach (var state in animator)
        {

        }
    }
}