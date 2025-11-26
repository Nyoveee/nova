using System;

class ElevatorButtonTest : Script
{
    [SerializableField]
    private Elevator elevator;
    protected override void update()
    {

        MapKey(Key.MouseLeft, () => {
        if(elevator.IsDoorClosed())
        { 
            if (elevator.IsAtTop())
                elevator.MoveElevatorDown();
            else
                elevator.MoveElevatorUp();
        }});

        MapKey(Key.MouseRight, () => {
            if (elevator.IsDoorOpen())
                elevator.CloseDoors();
            else if (elevator.IsDoorClosed())
                elevator.OpenDoors();
        });
    }
}