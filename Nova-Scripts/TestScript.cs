using ScriptingAPI;
namespace Nova_Scripts
{
    public class TestScript : Script
    {
        public override void Init()
        {
        
        }
        public override void update()
        {
            Transform_ transform = getComponent<Transform_>(); // Ideally, should be called once
            Vector3 position = transform.position;
            position.z += 1;
            transform.position = position;    
        }  
        public override void exit()
        {

        }
    }
}
