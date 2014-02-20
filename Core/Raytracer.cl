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


/* Here starts the raytracer*/
__kernel void Raytrace(__global float3* vertices,__global float3* normal, __global int3* faces,
						__global SceneInformation* sceneInfo, __global uchar3* frame) 
    { 
		int id = get_global_id(0);
		
		/* Using the syntax frame[x][y] produces different behaviour on different platforms (doesn't work on NVIDIA GPUS)
			So use the XYZ to access the members of any vector types */
		frame[id].x = 0;
		frame[id].y = 0;
		frame[id].z = 255;
    }