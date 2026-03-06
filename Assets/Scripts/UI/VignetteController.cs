// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class VignetteController : Script
{
  
    private float startFadeTime;
    private float currentFadeTime;
    private float startVignette;
    protected override void update()
    {
        if (RendererAPI.vignette == 0)
            return;
        currentFadeTime -= Time.V_FixedDeltaTime();
        currentFadeTime = Mathf.Max(currentFadeTime, 0);
        RendererAPI.vignette = Mathf.Interpolate(0f, startVignette, currentFadeTime / startFadeTime,1);
    }
    public void TriggerVignette(float vignetteAmount, float fadeTime, Colour vignetteColor)
    {
        RendererAPI.vignetteColor = vignetteColor;
        RendererAPI.vignette = startVignette = vignetteAmount;
        startFadeTime = currentFadeTime = fadeTime;
    }
}