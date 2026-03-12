// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

class VignetteController : Script
{
    enum VignetteState
    {
        FadeIn,
        FadeOut
    }
    private float startFadeTime;
    private float currentFadeTime;
    private float startVignette;
    private float targetVignette;
    private VignetteState vignetteState;
    protected override void update()
    {
        if (currentFadeTime == 0)
            return;
        currentFadeTime -= Time.V_FixedDeltaTime();
        currentFadeTime = Mathf.Max(currentFadeTime, 0);

        switch (vignetteState)
        {
            case VignetteState.FadeOut:
                RendererAPI.vignette = Mathf.Interpolate(0f, startVignette, currentFadeTime / startFadeTime, 1);
                break;
            case VignetteState.FadeIn:
                RendererAPI.vignette = Mathf.Interpolate(targetVignette, 0, currentFadeTime / startFadeTime, 1);
                break;
        }
    }
    public void TriggerVignetteFadeOut(float startVignette, float fadeTime, Colour vignetteColor)
    {
        RendererAPI.vignetteColor = vignetteColor;
        RendererAPI.vignette = this.startVignette = targetVignette;
        startFadeTime = currentFadeTime = fadeTime;
        vignetteState = VignetteState.FadeOut;
    }
    public void TriggerVignetteFadeIn(float targetVignette, float fadeTime, Colour vignetteColor)
    {
        RendererAPI.vignetteColor = vignetteColor;
        RendererAPI.vignette = this.targetVignette = targetVignette;
        startFadeTime = currentFadeTime = fadeTime;
        vignetteState = VignetteState.FadeIn;
    }
}