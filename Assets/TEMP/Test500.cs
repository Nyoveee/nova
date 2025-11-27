// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Test500 : Script
{
    public bool savingMode = false;
    // This function is first invoked when game starts.
    protected override void init()
    {
        if (savingMode)
        {
            //PlayerPrefs.SetInt("hello world!", 5);
            //PlayerPrefs.SetFloat("okay buddy", 45.25f);
            //PlayerPrefs.SetString("string", "hi chat 2");

            //PlayerPrefs.DeleteKey("okay buddy");
            PlayerPrefs.DeleteAll();
            PlayerPrefs.Save();
        }
        else
        {
            Debug.Log(PlayerPrefs.GetInt("hello world!"));
            Debug.Log(PlayerPrefs.GetFloat("okay buddy"));
            Debug.Log(PlayerPrefs.GetString("hello world!", "bismillah"));
        }
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}