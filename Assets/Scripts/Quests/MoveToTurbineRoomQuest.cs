// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class MoveToTurbineRoomQuest : MovementBasedQuest
{
    [SerializableField]
    private Door generatorExitDoor;

    [SerializableField]
    private Door closedRangedToHubDoor;

    [SerializableField]
    private Door hubToSewerDoor;

    [SerializableField]
    private GameObject sewerDoorLamp;

    [SerializableField]
    private GameObject closedRangedDoorLamp;

    [SerializableField]
    private float enabledLightIntensity = 19f;

    [SerializableField]
    private float disabledLightIntensity = 2f;

    [SerializableField]
    private Material enabledLampMaterial;

    [SerializableField]
    private Material disabledLampMaterial;

    public override void OnEnter()
    {
        generatorExitDoor.UnlockDoor();
        closedRangedToHubDoor.UnlockDoor();
        hubToSewerDoor.UnlockDoor();

        // enable door lamp to sewer..
        if (sewerDoorLamp != null)
        {
            sewerDoorLamp.getComponent<MeshRenderer_>().changeMaterial(0, enabledLampMaterial);

            foreach (GameObject child in sewerDoorLamp.GetChildren())
            {
                Light_ light = child.getComponent<Light_>();

                if (light != null)
                {
                    light.intensity = enabledLightIntensity;
                }
            }
        }

        // disable door lamp to sewer..
        if (closedRangedDoorLamp != null)
        {
            closedRangedDoorLamp.getComponent<MeshRenderer_>().changeMaterial(0, disabledLampMaterial);

            foreach (GameObject child in closedRangedDoorLamp.GetChildren())
            {
                Light_ light = child.getComponent<Light_>();

                if (light != null)
                {
                    light.intensity = disabledLightIntensity;
                }
            }
        }

    }

    public override void OnSuccess()
    {
    }

}