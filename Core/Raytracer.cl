/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2014, Henrique Jung, All rights reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
*/


/* enable double precision calculations on some OpenCL Compilers (NVIDIA) */
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define SMALL_NUM  0.00000001 // anything that avoids division overflow

/*Any change on those structs should be copied back to the host code on CLStructs.h */

/* Stores the concept of a Camera */
typedef struct Camera
{
	double3 pos;
	double3 dir;
	double3 lookAt;
	double3 up; // upvector
	double3 right;
}Camera;

/* Stores the concept of a light */
typedef struct Light
{
	double3 pos;
	double3 color;

	double Ks; // amount of specular
	double Ka; // amount of ambient
}Light;

/*SceneInformation struct holds important information related to the 3D scene and
how it should be rendered.*/
typedef struct SceneInformation
{
	int width;
	int height;
	int pixelCount;
	int verticesSize;
	int normalSize;
	int facesSize;
	int materiaslSize;
	double proportion_x;
	double proportion_y;
} SceneInformation;

/*Struct to control material properties */
typedef struct Material
{
	double3 ambientColor; //KA
	double3 diffuseColor; //KD
	double3 specularColor;//KS
}Material;



/* Möller–Trumbore intersection algorithm - http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm */
int Intersect( const double3   V1,  // Triangle vertices
				const double3   V2,
				const double3   V3,
				const double3    O,  //Ray origin
				const double3    D,  //Ray direction
				double3*	 normal,
				double3*	 point_i,
				double*	 dist,
						float* out )
{
	
	double3 e1, e2;  //Edge1, Edge2
  double3 P, Q, T;
  float det, inv_det, u, v;
  float t, t2;

  //Find intersection and distance
 
  //Find vectors for two edges sharing V1
  e1 = V2 - V1;
  e2 = V3 - V1;
  //Begin calculating determinant - also used to calculate u parameter, this is used to calculate normal as well, so we calculate here now
  P = cross(D, e2);
  *normal = cross(e1, e2);
  //if determinant is near zero, ray lies in plane of triangle
  det = dot(e1, P);
  //NOT CULLING
  if (det > - SMALL_NUM && det < SMALL_NUM) return 0;
  inv_det = 1.f / det;
 
  //calculate distance from V1 to ray origin
  T = O - V1;
  //*dist = length(T);
  //*point_i = (O)+(D) * (*dist);
 
  //Calculate u parameter and test bound
  u = dot(T, P) * inv_det;
  //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f) return 0;
 
  //Prepare to test v parameter
  Q = cross(T, e1);
 
  //Calculate V parameter and test bound
  v = dot(D, Q) * inv_det;
  //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f) return 0;
 
  t2 = dot(e2, Q);
  t = t2 * inv_det;
 
  if (t > SMALL_NUM) { //ray intersection

	  float r = t2 / det;			//if there was a intersection, compute distance!
	  *point_i = (O)+(D)* r; // intersect point of ray and plane
	  *dist = r;

    *out = t;
    return 1;
  }
 
  // No hit, no win
  return 0;


}

/* Here starts the raytracer*/
__kernel void Raytrace(__global double3* vertices, __global double3* normals, __global int4* faces, __global Material* materials,
	__global SceneInformation* sceneInfo, __global uchar4* frame, __global Camera* camera, __global Light* light)
{
	int id = get_global_id(0);
	// grab XY coordinate of this instance
	int x = id % sceneInfo->width;
	int y = id / sceneInfo->height;


	/* Using the syntax frame[x][y] produces different behaviour on different platforms (doesn't work on NVIDIA GPUS)
	So use the XYZ to access the members of any vector types */

	/* build direction of the ray based on camera and the current pixel */
	double normalized_i = -((double)((double)x / (double)(sceneInfo->width) * (double)(sceneInfo->proportion_x)) - 0.5);
	double normalized_j = -((double)((double)y / (double)(sceneInfo->height) * (double)(sceneInfo->proportion_y)) - 0.5);
	double3 ray_dir = (double3)(camera->right * normalized_i) + (double3)(camera->up * normalized_j) + camera->dir;
	
	ray_dir = normalize(ray_dir);

	double distance = 1000000.0; // high value for the first ray
	int face_i = -1; // index of the face that was hit, was -1 I don't now why
	double maxDistance = 1000000.0; //max distance, work as a far view point
	double3 point_i; // intersection point
	double3 normal; // face normal
	double3 l_origin = camera->pos; // local copy of origin of rays (camera/eye)
	float intersectOutput;
	// for each face, look for intersections with the ray
	for (unsigned int k = 0; k < sceneInfo->facesSize; k++)
	{
		int result;
		double3 temp_point; // temporary intersection point
		double3 temp_normal;// temporary normal vector

		result = Intersect(vertices[faces[k].x], vertices[faces[k].y], vertices[faces[k].z], l_origin, ray_dir, &temp_normal, &temp_point, &distance, &intersectOutput);

		if (result > 0)
		{
			//some collision
			if (distance < maxDistance) // check if it's the closest to the camera so far
			{
				maxDistance = distance;
				face_i = k;
				point_i = temp_point;
				normal = temp_normal;
			}
		}
	}
	// paint pixel
	if (face_i != -1)
	{

		// now that we have the face, calculate illumination
		double3 amount_color = (double3)(0.0, 0.0, 0.0); //final amount of color that goes to each pixel

		// get direction vector of light based on the intersection point
		double3 L = light->pos - point_i;
		L = normalize(L);
		normal = normalize(normal);

		int indexMaterial = faces[face_i].w;	// material is stored in the last component of the face vector

		//diffuse
		double dot_r = dot(normal, L);
		if (dot_r > 0)
		{
			double Kd = ((materials[indexMaterial].diffuseColor.x
				+ materials[indexMaterial].diffuseColor.y
				+ materials[indexMaterial].diffuseColor.z) / 3.0);
			double dif = dot_r * Kd;
			//put diffuse component
			amount_color += materials[indexMaterial].diffuseColor * light->color * dif;
		}
		//specular
		//glm::vec3 R = glm::cross(2.0f * glm::dot(L,normal) * normal,L);
		double3 R = L - 2.0 * dot(L, normal) * normal;
		dot_r = dot(ray_dir, R);
		if (dot_r > 0)
		{
			double spec = pown(dot_r, 20.0) * light->Ks;
			// put specular component
			amount_color += spec * light->color;
		}

		// build pixel
		double3 final_c;
		final_c.x = (amount_color.x) + (light->color.x * light->Ka); // put ambient
		final_c.y = (amount_color.y) + (light->color.y * light->Ka);
		final_c.z = (amount_color.z) + (light->color.z * light->Ka);

		if (final_c.x > 1.0)
			final_c.x = 1.0;
		if (final_c.y > 1.0)
			final_c.y = 1.0;
		if (final_c.z > 1.0)
			final_c.z = 1.0;

		frame[id].x = (final_c.x * 255.0);
		frame[id].y = (final_c.y * 255.0);
		frame[id].z = (final_c.z * 255.0);
		frame[id].w = 255; // full alpha
	}
	else
	{
		// no collision, put transparent pixel
		frame[id].x = 0;
		frame[id].y = 0;
		frame[id].z = 0;
		frame[id].w = 0; // zero alpha
	}
}