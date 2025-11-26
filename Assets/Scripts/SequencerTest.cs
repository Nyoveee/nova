// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class SequencerTest : Script
{
    private Sequence_ sequence;
    // This function is first invoked when game starts.
    protected override void init()
    {
        sequence = getComponent<Sequence_>();
        
        MapKey(Key.MouseLeft, () =>
        {
            sequence.play();
        });
        
        MapKey(Key.MouseRight, () =>
        {
            sequence.pause();
        });
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}