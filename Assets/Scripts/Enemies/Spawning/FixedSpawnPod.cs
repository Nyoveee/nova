// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

public class FixedSpawnPod : SpawnPod
{
    public void Spawn(Prefab enemyPrefab, Wave wave)
    {
        if (podTransform == null)
            podTransform = getComponent<Transform_>();

        gameObject.SetActive(true);
        GameObject enemy = Instantiate(enemyPrefab, podTransform.position);

        StartAnimation(enemy.transform);
        wave.RegisterSpawn(enemy);
    }

    protected override void OnAnimationFinished()
    {
        Destroy(gameObject);
    }
}