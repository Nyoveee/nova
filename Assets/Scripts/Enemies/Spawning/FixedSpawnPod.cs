// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

public class FixedSpawnPod : SpawnPod
{
    [SerializableField] 
    private Prefab enemyPrefab;

    public void Spawn()
    {
        if (podTransform == null)
            podTransform = getComponent<Transform_>();

        gameObject.SetActive(true);
        GameObject enemy = Instantiate(enemyPrefab, podTransform.position);
        StartAnimation(enemy.transform);
        if (wave != null)
            wave.RegisterSpawn(enemy);
    }

    protected override void OnAnimationFinished()
    {
        // Permanent to be reused
        gameObject.SetActive(false);
    }

    // Lerp need duration
    // Script for animation
}