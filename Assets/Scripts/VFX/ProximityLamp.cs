// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ProximityLamp : Script
{
    struct LightEntry
    {
        public LightEntry(Light_ light, float intiialIntensity)
        {
            this.light = light;
            this.initialIntensity = intiialIntensity;
        }

        public Light_ light;
        public float initialIntensity;
    }

    [SerializableField] private float distance = 40f;
    [SerializableField] private float dimLightIntensity = 1f;
    [SerializableField] private float dimLightEmissiveMultiplier = 0.5f;

    private Transform_ player;
    private List<LightEntry> lights = new ();

    private MeshRenderer_ meshRenderer;

    private bool playerIsClose = false;
    private float originalEmissiveStrength = 1f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        meshRenderer = getComponent<MeshRenderer_>();
        player = GameObject.FindWithTag("Player")?.transform;

        foreach (GameObject child in gameObject.GetChildren())
        {
            Light_ light = child.getComponent<Light_>();

            if (light != null)
            {
                lights.Add(new LightEntry( light, light.intensity ));
                light.intensity = dimLightIntensity;
            }
        }

        originalEmissiveStrength = meshRenderer.getMaterialFloat(0, "emissiveStrength");
        meshRenderer.setMaterialFloat(0, "emissiveStrength", dimLightEmissiveMultiplier);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (player == null)
        {
            return;
        }

        // Compare z distance proximity..
        float zDifference = Mathf.Abs(player.position.z - gameObject.transform.position.z);

        bool playerIsNowClose = zDifference < distance;
        
        // no change in state..
        if(playerIsNowClose == playerIsClose)
        {
            return;
        }

        playerIsClose = playerIsNowClose;

        meshRenderer.setMaterialFloat(0, "emissiveStrength", playerIsClose ? originalEmissiveStrength : dimLightEmissiveMultiplier);

        foreach (LightEntry light in lights)
        {
            light.light.intensity = playerIsClose ? light.initialIntensity : dimLightIntensity;
        }
    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}