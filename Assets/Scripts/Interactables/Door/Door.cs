using ScriptingAPI;
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

    private enum DoorOpeningMode
    {
        Translation,
        Rotation,
    }

    private delegate void CurrentState();
    private DoorState doorState = DoorState.Closed;
 
    private Dictionary<DoorState, CurrentState> updateState = new Dictionary<DoorState, CurrentState>();


    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private DoorType doorType;

    [SerializableField]
    private DoorOpeningMode doorOpeningMode;

    [SerializableField]
    private float automaticDoordetectionRange = 10f;
    // Left and right door panels
    [SerializableField]
    private Transform_ leftDoor;
    [SerializableField]
    private Transform_ rightDoor;
    [SerializableField]
    private Audio openSFX;
    [SerializableField]
    private Audio closeSFX;

    /***********************************************************
    Components
    ***********************************************************/
    private AudioComponent_ audioComponent;
    // Editable 
    [SerializableField]
    private float openOffset = 2f;

    [SerializableField]
    private float rotation = 90f;

    [SerializableField]
    private float doorMovingDuration = 2f;

    [SerializableField]
    private float lerpPower = 0.7f;

    // Positions
    private Vector3 leftStartClosed;
    private Vector3 rightStartClosed;
    private Vector3 leftStartOpen;
    private Vector3 rightStartOpen;
    private GameObject player;

    // Rotations..
    private Quaternion leftStartRotation;
    private Quaternion leftFinalRotation;

    private Quaternion rightStartRotation;
    private Quaternion rightFinalRotation;

    private float currentDoorMovingTime;
    protected override void init()
    {
        leftStartClosed = leftDoor.localPosition;
        rightStartClosed = rightDoor.localPosition;
        leftStartOpen = leftDoor.localPosition - new Vector3(openOffset, 0, 0);
        rightStartOpen = rightDoor.localPosition + new Vector3(openOffset, 0, 0);

        leftStartRotation = leftDoor.localRotation;
        rightStartRotation = rightDoor.localRotation;

        leftStartRotation = leftDoor.localRotation;
        rightStartRotation = rightDoor.localRotation;
        leftFinalRotation = Quaternion.AngleAxis(Mathf.Deg2Rad * -rotation, Vector3.Up()) * leftDoor.localRotation;
        rightFinalRotation = Quaternion.AngleAxis(Mathf.Deg2Rad * rotation, Vector3.Up()) * rightDoor.localRotation;

        player = GameObject.FindWithTag("Player");
        updateState.Add(DoorState.Open, Update_Open);
        updateState.Add(DoorState.Closed, Update_Closed);
        updateState.Add(DoorState.Opening, Update_Opening);
        updateState.Add(DoorState.Closing, Update_Closing);
        audioComponent = getComponent<AudioComponent_>();

        currentDoorMovingTime = 0f;
    }

    protected override void update()
    {
        if(player == null)
        {
            return;
        }

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
        currentDoorMovingTime += Time.V_DeltaTime();
        currentDoorMovingTime = Mathf.Min(doorMovingDuration, currentDoorMovingTime);

        float interval = currentDoorMovingTime / doorMovingDuration;

        switch (doorOpeningMode)
        {
            case DoorOpeningMode.Translation:
                leftDoor.localPosition = Vector3.Lerp(leftStartClosed, leftStartOpen, Mathf.Pow(interval, lerpPower));
                rightDoor.localPosition = Vector3.Lerp(rightStartClosed, rightStartOpen, Mathf.Pow(interval, lerpPower));

                break;
            case DoorOpeningMode.Rotation:
                leftDoor.localRotation = Quaternion.Slerp(leftStartRotation, leftFinalRotation, Mathf.Pow(interval, lerpPower));
                rightDoor.localRotation = Quaternion.Slerp(rightStartRotation, rightFinalRotation, Mathf.Pow(interval, lerpPower));
                break;
        }

        if (currentDoorMovingTime >= doorMovingDuration)
            doorState = DoorState.Open;
    }

    private void Update_Closing()
    {
        currentDoorMovingTime -= Time.V_DeltaTime();
        currentDoorMovingTime = Mathf.Max(currentDoorMovingTime, 0);

        float interval = currentDoorMovingTime / doorMovingDuration;

        switch (doorOpeningMode)
        {
            case DoorOpeningMode.Translation:
                leftDoor.localPosition = Vector3.Lerp(leftStartClosed, leftStartOpen, Mathf.Pow(interval, lerpPower));
                rightDoor.localPosition = Vector3.Lerp(rightStartClosed, rightStartOpen, Mathf.Pow(interval, lerpPower));
                
                break;
            case DoorOpeningMode.Rotation:
                leftDoor.localRotation = Quaternion.Slerp(leftStartRotation, leftFinalRotation, Mathf.Pow(interval, lerpPower));
                rightDoor.localRotation = Quaternion.Slerp(rightStartRotation, rightFinalRotation, Mathf.Pow(interval, lerpPower));

                break;
        }

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
        currentDoorMovingTime = 0;
        audioComponent?.PlaySound(openSFX);

        Debug.Log(openOffset);
    }

    public void CloseDoor()
    {
        doorState = DoorState.Closing;
        audioComponent?.PlaySound(closeSFX);
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