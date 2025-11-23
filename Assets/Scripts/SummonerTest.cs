// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System;
using System.Collections.Generic;
using System.Net.Http.Headers;
using Windows.Media.Protection.PlayReady;

class SummonerTest : Script
{
    //List<PathfindingCube> cubeList = new List<PathfindingCube>();
    bool [] array = new bool[5];
    PathfindingCube[]  cubeList = new PathfindingCube[5];

    [SerializableField]
    public Prefab cubePrefab;
    
    bool isShiftDown = false;
    private int slot = -1;
    private int index = -1;
    // This function is first invoked when game starts.
    protected override void init()
    {

        MapKey(Key._1, SetOne);
        MapKey(Key._2, SetTwo);
        MapKey(Key._3, SetThree);
        MapKey(Key._4, SetFour);

        MapKey(Key.LeftShift, ShiftDown, ShiftUp);
        MapKey(Key.MouseLeft, OnMouseClick);
        MapKey(Key.Delete, DeleteEnt);
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

        if (result != null && isShiftDown && slot != -1)
        {
            if (array[slot] == true) return;

            array[slot] = true;

            //Debug.Log("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
            GameObject cube = Instantiate(cubePrefab, result.Value.point, Quaternion.Identity(), null);

            //GameObject[] children = cube.GetChildren();

            //for(int i = 0; i < children.Length; i++)
            //{
            //    if (children[i].getScript<PathfindingCube>() != null)
            //    {
            //        children[i].getScript<PathfindingCube>().cubeIndex = cubeList.Count;
            //        cubeList.Add(children[i].getScript<PathfindingCube>());
            //    }
            //}
            cube.getScript<PathfindingCube>().cubeIndex = slot;
            cubeList[slot] = cube.getScript<PathfindingCube>();


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
        // if(index < 0 || index >= cubeList.Count) return;

        if (array[slot] == true)
        {

            Destroy(cubeList[slot].gameObject);
            array[slot] = false;
        }
    }


    public void SetOne()
    {

        slot = 0;
        for (int i = 0; i < cubeList.Length; i++)
        {
            if (i == slot)
            {
                if (array[i] == true)
                cubeList[i].isCubeControllable = true;
            }
            else
            {
                if (array[i] == true)
                cubeList[i].isCubeControllable = false;
            }
        }




    }

    public void SetTwo()
    {
        //for (int i = 0; i < cubeList.Count; i++)
        //{
        //    if (i == 1)
        //    {
        //        index = i;
        //        cubeList[i].isCubeControllable = true;
        //    }
        //    else
        //    {
        //        cubeList[i].isCubeControllable = false;
        //    }
        //}

        slot = 1;
        for (int i = 0; i < cubeList.Length; i++)
        {
            if (i == slot)
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = true;
            }
            else
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = false;
            }
        }

    }

    public void SetThree()
    {
        //for (int i = 0; i < cubeList.Count; i++)
        //{
        //    if (i == 2)
        //    {
        //        index = i;
        //        cubeList[i].isCubeControllable = true;
        //    }
        //    else
        //    {
        //        cubeList[i].isCubeControllable = false;
        //    }
        //}

        slot = 2;
        for (int i = 0; i < cubeList.Length; i++)
        {
            if (i == slot)
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = true;
            }
            else
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = false;
            }
        }


    }


    public void SetFour()
    {
        //for (int i = 0; i < cubeList.Count; i++)
        //{
        //    if (i == 3)
        //    {
        //        index = i;
        //        cubeList[i].isCubeControllable = true;
        //    }
        //    else
        //    {
        //        cubeList[i].isCubeControllable = false;
        //    }
        //}

        slot = 3;
        for (int i = 0; i < cubeList.Length; i++)
        {
            if (i == slot)
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = true;
            }
            else
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = false;
            }
        }
    }

    public void SetFive()
    {
        //for (int i = 0; i < cubeList.Count; i++)
        //{
        //    if (i == 3)
        //    {
        //        index = i;
        //        cubeList[i].isCubeControllable = true;
        //    }
        //    else
        //    {
        //        cubeList[i].isCubeControllable = false;
        //    }
        //}

        slot = 4;
        for (int i = 0; i < cubeList.Length; i++)
        {
            if (i == slot)
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = true;
            }
            else
            {
                if (array[i] == true)
                    cubeList[i].isCubeControllable = false;
            }
        }
    }
}