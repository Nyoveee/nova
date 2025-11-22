// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Collections.Generic;
using System.Net.Http.Headers;

class SummonerTest : Script
{
    List<PathfindingCube> cubeList = new List<PathfindingCube>();

    [SerializableField]
    public Prefab cubePrefab;
    
    bool isShiftDown = false;

    private int index = 0;
    // This function is first invoked when game starts.
    protected override void init()
    {

        MapKey(Key._1, SetOne);
        MapKey(Key._2, SetTwo);
        MapKey(Key._3, SetThree);
        MapKey(Key._4, SetFour);

        MapKey(Key.LeftShift, ShiftDown, ShiftUp);
        MapKey(Key.MouseLeft, OnMouseClick);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        // insta



    }

    private void OnMouseClick()
    {
        //ray cast to a position to spawn cube
        Ray ray = CameraAPI.getRayFromMouse();
        RayCastResult? result = PhysicsAPI.Raycast(ray, 1000f);

        if (result != null && isShiftDown)
        {
            //Debug.Log("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
            GameObject cube = Instantiate(cubePrefab, result.Value.point, Quaternion.Identity(), null);

            GameObject[] children = cube.GetChildren();

            for(int i = 0; i < children.Length; i++)
            {
                if (children[i].getScript<PathfindingCube>() != null)
                {
                    children[i].getScript<PathfindingCube>().cubeIndex = cubeList.Count;
                    cubeList.Add(children[i].getScript<PathfindingCube>());
                }
            }

            //cube.getScript<PathfindingCube>().cubeIndex = cubeList.Count;
            //cubeList.Add(cube.getScript<PathfindingCube>());


        }
    }


    public void ShiftDown()
    {
        isShiftDown = true;
    }

    public void ShiftUp()
    {

        isShiftDown = false;
    }


    public void DeleteEnt()
    {
        cubeList.RemoveAt(index);  
    
    }


    public void SetOne()
    { 
        for(int i = 0; i < cubeList.Count; i++)
        { 
            if (i == 0)
            {
                index = i;
                cubeList[i].isCubeControllable = true;
            }
            else
            {
                cubeList[i].isCubeControllable = false;
            }
        }
    
    
    }

    public void SetTwo()
    {
        for (int i = 1; i < cubeList.Count; i++)
        {
            if (i == 1)
            {
                index = i;
                cubeList[i].isCubeControllable = true;
            }
            else
            {
                cubeList[i].isCubeControllable = false;
            }
        }


    }

    public void SetThree()
    {
        for (int i = 2; i < cubeList.Count; i++)
        {
            if (i == 2)
            {
                index = i;
                cubeList[i].isCubeControllable = true;
            }
            else
            {
                cubeList[i].isCubeControllable = false;
            }
        }


    }


    public void SetFour()
    {
        for (int i = 3; i < cubeList.Count; i++)
        {
            if (i == 3)
            {
                index = i;
                cubeList[i].isCubeControllable = true;
            }
            else
            {
                cubeList[i].isCubeControllable = false;
            }
        }


    }
}