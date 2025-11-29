// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

public class RandomSpawnPod : SpawnPod
{
    private Prefab enemyPrefab;

    public void InitValues(Wave w, Prefab prefab)
    {
#if false
        wave = w;
        enemyPrefab = prefab;
        
        GameObject enemy = Instantiate(enemyPrefab, podTransform.position);
        StartAnimation(enemy.transform);
        if (wave != null)
            wave.RegisterSpawn(enemy);
#endif
    }

    protected override void OnAnimationFinished()
    {
        // Temporary, destroyed upon completion
        Destroy(gameObject);
    }
}