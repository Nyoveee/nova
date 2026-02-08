// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SettingsButtonScript : Script
{
    [SerializableField]
    Image_ yesImage;
    [SerializableField]
    Image_ noImage;
    [SerializableField]
    private String preference;
    protected override void init()
    {
        bool enabled = false;
        try{
            enabled = Convert.ToBoolean(PlayerPrefs.GetInt(preference));
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
        noImage.colorTint = new ColorAlpha(1, 1, 1, 0.5f);
        PlayerPrefs.SetInt(preference, 1);
    }
    public void NoEnabled()
    {
        yesImage.colorTint = new ColorAlpha(1, 1, 1, 0.5f);
        noImage.colorTint = new ColorAlpha(1, 1, 1, 1);
        PlayerPrefs.SetInt(preference, 0);
    }

}