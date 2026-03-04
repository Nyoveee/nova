// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class PulsatingEmissive : Script
{
    [SerializableField]
    private SkinnedMeshRenderer_ skinnedMeshRenderer_;
    [SerializableField]
    private float speedMultiplier;
    [SerializableField]
    private float maxEmissiveStrength;

    private float timeElapsed;
    protected override void update()
    {
        timeElapsed += Time.V_DeltaTime() * speedMultiplier;

        // loop..
        timeElapsed %= 360 * Mathf.Deg2Rad;

        float sine = Mathf.Sin(timeElapsed);

        // remap range..
        sine = (sine + 1) / 2;

        skinnedMeshRenderer_.setMaterialFloat(0, "emissiveStrength", Mathf.Interpolate(0, maxEmissiveStrength, sine, 1f));
    }   
}