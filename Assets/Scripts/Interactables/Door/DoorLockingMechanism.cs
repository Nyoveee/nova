// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class DoorLockingMechanism : Script
{
    [SerializableField]
    private Door door;

    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player" && door.IsDoorUnlocked())
        {
            Destroy(gameObject);
            door.LockDoor();
        }
    }

}