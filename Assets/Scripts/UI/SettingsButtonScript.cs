// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

enum RenderSettings
{
    Fullscreen,
    AntiAliasing,
    SSAO,
    Shadows,
    Fog
};

class SettingsButtonScript : Script
{
    [SerializableField]
    private RenderSettings preference;

    private bool enabled = false;
    private Button_ fill;

    protected override void init()
    {
        enabled = GetConfigStatus(preference);

        fill = getComponent<Button_>();

        if (enabled)
            Enable();
        else
            Disable();

        if (!enabled)
            // we overwrite normal color above, and because button is not hovering, we need to explicitly update the color..
            fill.forceColorUpdate();
    }

    public void onReleased()
    {
        enabled = !enabled;

        if (enabled)
        {
            Enable();
            fill.highlightedColor = new ColorAlpha(fill.highlightedColor.r, fill.highlightedColor.g, fill.highlightedColor.b, 1);
        }
        else
        {
            Disable();
            fill.highlightedColor = new ColorAlpha(fill.highlightedColor.r, fill.highlightedColor.g, fill.highlightedColor.b, 0);
        }
    }

    public void onHover(){}

    public void onHoverLeave() 
    {
        if (!enabled)
            fill.highlightedColor = new ColorAlpha(fill.highlightedColor.r, fill.highlightedColor.g, fill.highlightedColor.b, 1);
    }

    public void Enable()
    {
        SetConfigStatus(preference, true);
        fill.normalColor = new ColorAlpha(1, 1, 1, 1);
    }

    public void Disable()
    {
        SetConfigStatus(preference, false);
        fill.normalColor = new ColorAlpha(0, 0, 0, 0);
    }

    public bool GetConfigStatus(RenderSettings renderSettings)
    {
        switch (renderSettings)
        {
            case RenderSettings.Fullscreen:
                return RendererAPI.fullScreen;
                break;
            case RenderSettings.AntiAliasing:
                return RendererAPI.antiAliasingEnabled;
                break;
            case RenderSettings.SSAO:
                return RendererAPI.ssaoEnabled;
                break;
            case RenderSettings.Shadows:
                return RendererAPI.shadowsEnabled;
                break;
            case RenderSettings.Fog:
                return RendererAPI.fogEnabled;
                break;
            default:
                return false;
        }
    }

    public void SetConfigStatus(RenderSettings renderSettings, bool value)
    {
        switch (renderSettings)
        {
            case RenderSettings.Fullscreen:
                RendererAPI.fullScreen = value;
                break;
            case RenderSettings.AntiAliasing:
                RendererAPI.antiAliasingEnabled = value; 
                break;
            case RenderSettings.SSAO:
                RendererAPI.ssaoEnabled = value;
                break;
            case RenderSettings.Shadows:
                RendererAPI.shadowsEnabled = value;
                break;
            case RenderSettings.Fog:
                RendererAPI.fogEnabled = value;
                break;
        }
    }
}