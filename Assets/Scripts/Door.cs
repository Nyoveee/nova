using System;

class Door : Script
{
    private enum DoorState
    {
        Closed,
        Opening,
        Open,
        Closing
    }
    private delegate void CurrentState();
    private DoorState doorState = DoorState.Closed;
    private Dictionary<DoorState, CurrentState> updateState = new Dictionary<DoorState, CurrentState>();


    // Left and right door panels
    [SerializableField]
    private Transform_ leftDoor;
    [SerializableField]
    private Transform_ rightDoor;

    // Editable 
    [SerializableField]
    private float openOffset = 2f;
    [SerializableField]
    private float doorMovingDuration = 2f;

    // Positions
    private Vector3 leftStart;

    private Vector3 rightStart;

 
    private float currentDoorMovingTime;
    protected override void init()
    {
        updateState.Add(DoorState.Open, Update_Open);
        updateState.Add(DoorState.Closed, Update_Closed);
        updateState.Add(DoorState.Opening, Update_Opening);
        updateState.Add(DoorState.Closing, Update_Closing);
    }
    protected override void update()
    {
        updateState[doorState]();
    }
    private void Update_Open() { }
    private void Update_Closed() { }
    private void Update_Locked() { }
    private void Update_Opening()
    {
        currentDoorMovingTime -= Time.V_DeltaTime();

        leftDoor.localPosition = Vector3.Lerp(leftStart + new Vector3(0, 0, openOffset), leftStart, currentDoorMovingTime/doorMovingDuration);
        rightDoor.localPosition = Vector3.Lerp(rightStart - new Vector3(0, 0, openOffset),rightStart, currentDoorMovingTime / doorMovingDuration);

        if (currentDoorMovingTime <= 0f)
            doorState = DoorState.Open;
    }
    private void Update_Closing()
    {
        currentDoorMovingTime -= Time.V_DeltaTime();

        leftDoor.localPosition = Vector3.Lerp(leftStart - new Vector3(0, 0, openOffset), leftStart, currentDoorMovingTime / doorMovingDuration);
        rightDoor.localPosition = Vector3.Lerp(rightStart + new Vector3(0, 0, openOffset), rightStart, currentDoorMovingTime / doorMovingDuration);

        if (currentDoorMovingTime <= 0f)
            doorState = DoorState.Closed;
    }
    public bool IsFullyOpened() => doorState == DoorState.Open;
    public bool IsFullyClosed() => doorState == DoorState.Closed;
    /**********************************************************************
        Collision Trigger Events
    **********************************************************************/
    public void OpenDoor()
    {
        doorState = DoorState.Opening;
        leftStart = leftDoor.localPosition;
        rightStart = rightDoor.localPosition;
        currentDoorMovingTime = doorMovingDuration;
    }
    public void CloseDoor()
    {
        doorState = DoorState.Closing;
        leftStart = leftDoor.localPosition;
        rightStart = rightDoor.localPosition;
        currentDoorMovingTime = doorMovingDuration;
    }
}