// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

public class GeneratorArenaQuest : ArenaQuest
{
    [SerializableField]
    private ArenaManager arenaManager;

    [SerializableField]
    private GameObject ambientPointLights;

    [SerializableField]
    private GameObject doorLamp;

    [SerializableField]
    private GameObject generatorSwitchLight;

    [SerializableField]
    private Material enabledLampMaterial;

    public override void OnEnter()
    {
        arenaManager.StartArena(this);

        // off the pulsating light switch..
        generatorSwitchLight?.SetActive(false);

        // enable door lamp to sewer..
        if (ambientPointLights != null)
        {
            foreach (GameObject child in ambientPointLights.GetChildren())
            {
                child.SetActive(true);
            }
        }

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
    }

    public override void UpdateQuest()
    {
    }
}