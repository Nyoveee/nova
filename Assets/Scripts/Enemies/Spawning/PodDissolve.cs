// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System;

class PodDissolve : Script
{
    MeshRenderer_ podRenderer;
    private bool dissolving = false;
    private float dissolveTimer = 0f;
    private float dissolveDuration = 1f;

    // This function is first invoked when game starts.
    protected override void init()
    {
        podRenderer = getComponent<MeshRenderer_>();
        podRenderer.setMaterialFloat(2, "dissolveThreshold", 1f);
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (!dissolving || podRenderer == null) return;

        dissolveTimer += Time.V_DeltaTime();
        float ratio = dissolveTimer / dissolveDuration;
        podRenderer.setMaterialFloat(0, "dissolveThreshold", 1f - ratio);
        podRenderer.setMaterialFloat(1, "dissolveThreshold", 1f - ratio);
        podRenderer.setMaterialFloat(2, "dissolveThreshold", (1f - ratio) * (1f - ratio));
        podRenderer.setMaterialFloat(3, "dissolveThreshold", 1f - ratio);

        if (ratio >= 1f)
        {
            dissolving = false;
            Destroy(gameObject);
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    public void StartDissolve(float duration = 1f)
    {
        dissolveDuration = duration;
        dissolveTimer = 0f;
        dissolving = true;
    }

}