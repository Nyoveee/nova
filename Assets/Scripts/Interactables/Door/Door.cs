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
    private enum DoorType
    {
        Elevator,
        Automatic,
        Locked
    }
    private delegate void CurrentState();
    private DoorState doorState = DoorState.Closed;
 
    private Dictionary<DoorState, CurrentState> updateState = new Dictionary<DoorState, CurrentState>();


    [SerializableField]
    private DoorType doorType;
    [SerializableField]
    private float automaticDoordetectionRange = 10f;
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
    private Vector3 leftStartClosed;
    private Vector3 rightStartClosed;
    private Vector3 leftStartOpen;
    private Vector3 rightStartOpen;
    private GameObject player;

 
    private float currentDoorMovingTime;
    protected override void init()
    {
        leftStartClosed = leftDoor.localPosition;
        rightStartClosed = rightDoor.localPosition;
        leftStartOpen = leftDoor.localPosition - new Vector3(openOffset, 0, 0);
        rightStartOpen = rightDoor.localPosition + new Vector3(openOffset, 0, 0);
        player = GameObject.FindWithTag("Player");
        updateState.Add(DoorState.Open, Update_Open);
        updateState.Add(DoorState.Closed, Update_Closed);
        updateState.Add(DoorState.Opening, Update_Opening);
        updateState.Add(DoorState.Closing, Update_Closing);
    }
    protected override void update()
    {
        switch (doorType)
        {
            // Door will open depending on player 
            case DoorType.Automatic:
                if(Vector3.Distance(player.transform.position,gameObject.transform.position) <= automaticDoordetectionRange)
                {
                    if (doorState != DoorState.Open && doorState != DoorState.Opening)
                        OpenDoor();
                }
                else
                {
                    if (doorState != DoorState.Closed && doorState != DoorState.Closing)
                        CloseDoor();
                }
                updateState[doorState]();
                break;
            // Force door to be closed
            case DoorType.Locked:
                if (doorState != DoorState.Closed && doorState != DoorState.Closing)
                    CloseDoor();
                updateState[doorState]();
                break;
            // Setting will be done manually by the elevator
            case DoorType.Elevator:
                updateState[doorState]();
                break;
        }
        
    }
    private void Update_Open() { }
    private void Update_Closed() { }
    private void Update_Locked() { }
    private void Update_Opening()
    {
        currentDoorMovingTime -= Time.V_DeltaTime();
        currentDoorMovingTime = Mathf.Max(currentDoorMovingTime, 0f);
        leftDoor.localPosition = Vector3.Lerp(leftStartOpen, leftStartClosed, currentDoorMovingTime/doorMovingDuration);
        rightDoor.localPosition = Vector3.Lerp(rightStartOpen, rightStartClosed, currentDoorMovingTime / doorMovingDuration);

        if (currentDoorMovingTime <= 0f)
            doorState = DoorState.Open;
    }
    private void Update_Closing()
    {
        currentDoorMovingTime -= Time.V_DeltaTime();
        currentDoorMovingTime = Mathf.Max(currentDoorMovingTime, 0f);
        leftDoor.localPosition = Vector3.Lerp(leftStartClosed, leftStartOpen, currentDoorMovingTime / doorMovingDuration);
        rightDoor.localPosition = Vector3.Lerp(rightStartClosed, rightStartOpen, currentDoorMovingTime / doorMovingDuration);

        if (currentDoorMovingTime <= 0f)
            doorState = DoorState.Closed;
    }
    public bool IsFullyOpened() => doorState == DoorState.Open;
    public bool IsFullyClosed() => doorState == DoorState.Closed;
    public bool IsDoorUnlocked() => (DoorType)doorType == DoorType.Automatic;
    /**********************************************************************
        Manual Events
    **********************************************************************/
    public void OpenDoor()
    {
        doorState = DoorState.Opening;

        currentDoorMovingTime = doorMovingDuration;
        // AudioAPI.PlaySound(gameObject, "slidingDoor_open_01");
    }
    public void CloseDoor()
    {
        doorState = DoorState.Closing;

        currentDoorMovingTime = doorMovingDuration;
        // AudioAPI.PlaySound(gameObject, "slidingDoor_close_01");
    }
    public void LockDoor()
    {
        doorType = DoorType.Locked;
    }
    public void UnlockDoor()
    {
        doorType = DoorType.Automatic;
    }
}