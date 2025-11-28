// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public abstract class WaveBehavior : Script
{
    protected Wave wave;

    public virtual void Init(Wave w)
    {
        wave = w;
    }

    public abstract void StartWave();
    public abstract void UpdateWave(int aliveCount);
    public abstract bool IsWaveComplete(int aliveCount);
}