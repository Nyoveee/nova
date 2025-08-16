using ScriptingAPI;
using System;

namespace Nova_Scripts
{
    public class TestScript : Script
    {
        Transform_ transform;

        public override void Init()
        {
            transform = getComponent<Transform_>();
        }

        public override void update()
        {
            Vector3 position = transform.position;
            position.z += (float) 0.01;
            transform.position = position;
        }  

        public override void exit()
        {

        }
    }
}
