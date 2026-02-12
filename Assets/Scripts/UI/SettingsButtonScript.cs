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
    Image_ yesImage;
    
    [SerializableField]
    Image_ noImage;
    
    [SerializableField]
    private RenderSettings preference;
    protected override void init()
    {
        bool enabled = false;
        try{
            enabled = GetConfigStatus(preference);
        }
        catch{}
            
        if (enabled)
            YesEnabled();
        else 
            NoEnabled();
    }
    public void onPressed(){}

    public void onHover(){}

    public void YesEnabled()
    {
        yesImage.colorTint = new ColorAlpha(1, 1, 1, 1);
        noImage.colorTint = new ColorAlpha(0.5f, 0.5f, 0.5f, 0.5f);

        SetConfigStatus(preference, true);
    }
    public void NoEnabled()
    {
        yesImage.colorTint = new ColorAlpha(0.5f, 0.5f, 0.5f, 0.5f);
        noImage.colorTint = new ColorAlpha(1, 1, 1, 1);

        SetConfigStatus(preference, false);
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