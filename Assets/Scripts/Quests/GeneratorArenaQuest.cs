// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Graphics.Display;

public class GeneratorArenaQuest : ArenaQuest
{
    [SerializableField]
    private ArenaManager arenaManager;

    [SerializableField]
    private GameObject ambientPointLights;

    [SerializableField]
    private float ambientPointLightIntensity = 20f;

    [SerializableField]
    private List<Light_> shadowCasters = new List<Light_>();

    [SerializableField]
    private float shadowCasterIntensity = 800f;

    [SerializableField]
    private GameObject doorLamp;

    [SerializableField]
    private Light_ generatorSwitchLight;

    [SerializableField]
    private Material enabledLampMaterial;

    [SerializableField]
    private float lightUpDuration = 0.3f;

    [SerializableField]
    private float lerpPower = 1f;

    private bool isAnimating = false;
    private bool isAnimatingGeneratorSwitch = false;

    private float timeElapsed = 0f;

    private float switchTimeElapsed = 0f;

    private float initialSwitchLightIntensity = 0f;
    private float initialShadowCasterIntensity = 0f;

    public override void OnEnter()
    {
        if (generatorSwitchLight != null)
        {
            initialSwitchLightIntensity = generatorSwitchLight.intensity;
            generatorSwitchLight.gameObject.getScript<PulsatingLight>().isActive = false;
            isAnimatingGeneratorSwitch = true;
        }

        Invoke(() =>
        {
            arenaManager.StartArena(this);

            if (shadowCasters.Count > 0)
            {
                initialShadowCasterIntensity = shadowCasters[0].intensity;
            }

            isAnimating = true;
        }, 1f);
    }

    public override void OnSuccess()
    {
        // enable door lamp to sewer..
        if (doorLamp != null)
        {
            doorLamp.getComponent<MeshRenderer_>().changeMaterial(0, enabledLampMaterial);

            foreach (GameObject child in doorLamp.GetChildren())
            {
                child.SetActive(true);
            }
        }

        foreach (Light_ shadowCaster in shadowCasters)
        {
            shadowCaster.gameObject.SetActive(false);
        }

        // off shadow caster and ambient light..
        if (ambientPointLights != null)
        {
            foreach (GameObject child in ambientPointLights.GetChildren())
            {
                child.SetActive(false);
            }
        }

    }

    public override void UpdateQuest()
    {
        if(isAnimatingGeneratorSwitch)
        {
            switchTimeElapsed += Time.V_DeltaTime();
            float interval = Mathf.Min(switchTimeElapsed / lightUpDuration, 1f);

            // off the pulsating light switch..
            if (generatorSwitchLight != null)
            {
                generatorSwitchLight.intensity = Mathf.Interpolate(initialSwitchLightIntensity, 0f, interval, lerpPower);
            }

            if (switchTimeElapsed > lightUpDuration)
            {
                isAnimatingGeneratorSwitch = false;
            }
        }

        if (isAnimating)
        {
            timeElapsed += Time.V_DeltaTime();
            float interval = Mathf.Min(timeElapsed / lightUpDuration, 1f);

            // enable ambient light
            if (ambientPointLights != null)
            {
                foreach (GameObject child in ambientPointLights.GetChildren())
                {
                    Light_ light = child.getComponent<Light_>();
                    
                    if(light!= null)
                    {
                        light.intensity = Mathf.Interpolate(0f, ambientPointLightIntensity, interval, lerpPower);
                    }
                }
            }

            // increase shadow caster's intensity..
            foreach (Light_ shadowCaster in shadowCasters)
            {
                shadowCaster.intensity = Mathf.Interpolate(initialShadowCasterIntensity, shadowCasterIntensity, interval, lerpPower);
            }

            if(timeElapsed > lightUpDuration)
            {
                isAnimating = false;
            }
        }
    }
}