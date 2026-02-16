// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class LaunchingVFX : Script
{
    [SerializableField]
    private GameObject landing;
    [SerializableField]
    private GameObject air;
    [SerializableField]
    private Light_ light;
    [SerializableField]
    private ParticleEmitter_ landingFireEmitter;

    private Enemy enemy;

    private float currentBurstEmittertime;
    protected override void update()
    {
        if(enemy!= null && enemy.IsTouchingGround() && air.IsActive())
        {
            air.SetActive(false);
            float longestBurstEmitterTime = 0;
            foreach (GameObject emitterGO in landing.GetChildren())
            {
                ParticleEmitter_ emitter = emitterGO.getComponent<ParticleEmitter_>();
                emitter.emit();
                longestBurstEmitterTime = (emitter.lifeTime > longestBurstEmitterTime)? emitter.lifeTime : longestBurstEmitterTime;
            }
            Invoke(() =>
            {
                if (gameObject != null)
                    Destroy(gameObject);
            }, longestBurstEmitterTime);
        }
        if (!air.IsActive())
        {
            currentBurstEmittertime += Time.V_DeltaTime();
            if (currentBurstEmittertime > landingFireEmitter.lifeTime)
                light.enable = false;
        }
    }
    public void SetEnemy(Enemy enemy){
        this.enemy = enemy;
        air.transform.localPosition += new Vector3(0, enemy.gameObject.transform.scale.y / 2f, 0);
    }

    



}