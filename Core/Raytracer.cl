/*
	RenderGirl - OpenCL raytracer renderer
	Copyright(C) Henrique Jung

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define SMALL_NUM  0.00000001f // anything that avoids division overflow

/*Any change on those structs should be copied back to the host code on CLStructs.h */

/* Stores the concept of a Camera */
typedef struct Camera
{
	float3 pos;
	float3 dir;
	float4 screenCoordinates; //defines where the screen begins in world space
	// and some math stuff to help calculate rays
	float delta_x;
    float delta_y; 
    float interpolation_x; 
    float interpolation_y;
}Camera;

/*SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.*/ 
typedef struct SceneInformation
{
	int resolution;
	int pixelCount;
	int verticesSize;
	int normalSize;
	int facesSize;
} SceneInformation;


/* Intersect function test collision with triangles only, based on a function provided by Rossana Baptista Queiroz */
int Intersect(float* dist, float3* origin, float3* dir,float3* point,int indexFace,float3* normal, 
				__global int3* faces, __global float3* vertices)
{
	// get position of the three vertices of the triangle
	float3 tri[3];
	
	// first vertex
	tri[0].x = vertices[faces[indexFace].x].x;
	tri[0].y = vertices[faces[indexFace].x].y;
	tri[0].z = vertices[faces[indexFace].x].z;
    // second vertex
    tri[1].x = vertices[faces[indexFace].y].x;
	tri[1].y = vertices[faces[indexFace].y].y;
	tri[1].z = vertices[faces[indexFace].y].z;
    // third vertex
    tri[2].x = vertices[faces[indexFace].z].x;
	tri[2].y = vertices[faces[indexFace].z].y;
	tri[2].z = vertices[faces[indexFace].z].z;

    float3 u,v,n; //triangle vectors
    float3 w0,w; //ray vectors
    float r,a,b; // params to calc ray-plane intersect
    
    // get triangle edge vectors and plane normal
	u = tri[1] - tri[0];
	v = tri[2] - tri[0];
	*normal = cross(u,v);
	float3 zero = (float3)(0.0,0.0,0.0);
	//if (all(*normal == zero))
	//	return -1; // triangle is degenerate, not deal with this case
					
	w0 = (*origin) - tri[0];
	a = -dot(*normal,w0);
	b = dot(*normal,*dir);
	if (fabs(b) < SMALL_NUM) // ray is parallel to triangle plane
	{	
		if (a == 0)				// ray lies in triangle plane
			return 2;
		else return 0;	// ray disjoint from plane
	}
	
	
	// get intersection point of ray with triangle plane
	r = a / b;
	if (r < 0.0f) // ray goes away from triangle
		return 0; // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
	
	*point = (*origin) + (*dir) * r; // intersect point of ray and plane
	
	*dist = r;
	
	// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	uu = dot(u,u);
	uv = dot(u,v);
	vv = dot(v,v);
	w = (*point) - tri[0];
	wu = dot(w,u);
	wv = dot(w,v);
	D = uv * uv - uu * vv;
	
	// get and test parametric coords
    float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)        // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                      // I is in T

}

/* Here starts the raytracer*/
__kernel void Raytrace(__global float3* vertices,__global float3* normal, __global int3* faces,
						__global SceneInformation* sceneInfo, __global uchar3* frame, __global Camera* camera_p) 
    { 
		int id = get_global_id(0);
		// grab XY coordinate of this instance
		int x = id % sceneInfo->resolution;
		int y = id / sceneInfo->resolution;
		
		/* Using the syntax frame[x][y] produces different behaviour on different platforms (doesn't work on NVIDIA GPUS)
			So use the XYZ to access the members of any vector types */
		
		
		/*compute some camera stuff */
		// define observer
		Camera camera;
		camera.dir = (float3)(0.0,0.0,0.0);
		camera.pos = (float3)(0.0,0.0,-10.0);
		// compute values for shooting the rays
		// screen plane in world space coordinates
		camera.screenCoordinates.x = -4;
		camera.screenCoordinates.y = 4;
		camera.screenCoordinates.z = camera.interpolation_y = 4;
		camera.screenCoordinates.w = -4;
		// calculate deltas for interpolation
		camera.delta_x = (camera.screenCoordinates.y - camera.screenCoordinates.x) / sceneInfo->resolution;
		camera.delta_y = (camera.screenCoordinates.w - camera.screenCoordinates.z) / sceneInfo->resolution;
		camera.interpolation_y += 20 * camera.delta_y;
		
		/* calculate interpolation for this pixel */
		camera.interpolation_x += camera.delta_x * x;
		camera.interpolation_y += camera.delta_y * y;
		
		/*TEMP SHIT*/
		camera_p[id].interpolation_x = camera.interpolation_x;
		camera_p[id].interpolation_y = camera.interpolation_y;
		camera_p[id].delta_x = camera.delta_x;
		camera_p[id].delta_y = camera.delta_y;
		camera_p[id].pos = camera.pos;
		camera_p[id].dir = camera.dir;
		camera_p[id].screenCoordinates = camera.screenCoordinates;
		
		/* build direction of the ray based on camera and the current pixel*/
		float3 ray_dir = (float3)(camera.interpolation_x,camera.interpolation_y,0) - camera.pos;
		ray_dir = normalize(ray_dir);
		
		float distance = 1000000.0f; // high value for the first ray
		int face_i = -1; // index of the face that was hit
		float maxDistance = 1000000.0f; //max distance, work as a far view point
	    float3 point_i; // intersection point
	    float3 l_normal; // face normal
		// for each face, look for intersections with the ray
		for (unsigned int k = 0; k < sceneInfo->facesSize; k++)
		{
			int result;
			float3 temp_point; // temporary intersection point
			float3 temp_normal;// temporary normal vector
			
           result = Intersect(&distance,&camera.pos,&ray_dir,&temp_point,k,&temp_normal,faces,vertices);
		   
		   if (result > 0) 
		   {
				//some collision
				if (distance < maxDistance) // check if it's the closest to the camera so far
				{
					maxDistance = distance;
					face_i = k;
					point_i = temp_point;
					l_normal = temp_normal;
				}
		   }
		}
		// paint pixel
		if (face_i != -1)
		{
			// collision, put white pixel
			frame[id].x = 255;
			frame[id].y = 255;
			frame[id].z = 255;
		}
		else
		{
			// no collision, put black pixel
			frame[id].x = 0;
			frame[id].y = 0;
			frame[id].z = 0;
		}
		
    }