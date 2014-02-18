 
/*SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.
	Any change on this struct should be copied back to the host code on CLStructs.h
*/ 
typedef struct SceneInformation
{
	int resolution;
	int pixelCount;
	int verticesSize;
	int normalSize;
	int facesSize;
} SceneInformation;


/* Here start the raytracer*/
__kernel void Raytrace(__global float3* vertices,__global float3* normal, __global int3* faces,
						__global SceneInformation* sceneInfo, __global char3* frame) 
    { 
		int i = vertices[0][0] * vertices[0][1];
		
		int id = get_global_id(0);
		frame[id][0] = 128;
		frame[id][1] = 200;
		frame[id][2] = 203;
    }