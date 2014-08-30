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

#define SMALL_NUM  0.00000001f // anything that avoids division overflow

/*Any change on those structs should be copied back to the host code on CLStructs.h */

/* Stores the concept of a Camera */
typedef struct Camera
{
	float3 pos;
	float3 dir;
	float3 lookAt;
	float3 up; // upvector
	float3 right;
}Camera;

/* Stores the concept of a light */
typedef struct Light
{
	float3 pos;
	float3 color;

	float Ks; // amount of specular
	float Ka; // amount of ambient
}Light;

/*SceneInformation struct holds important information related to the 3D scene and
how it should be rendered.*/
typedef struct SceneInformation
{
	int width;
	int height;
	int pixelCount;
	int groupsSize;
	float proportion_x;
	float proportion_y;
} SceneInformation;

/* SceneGroup struct holds info about a particular scene group */
typedef struct SceneGroupStruct
{
	int facesSize; /* amount of faces of this particular group */
	int facesStart;/* the index where the faces of this group start inside the global faces buffer */
	float sphereSize; /* bouding sphere size */
	float3 spherePos; /* bouding sphere position */
}SceneGroupStruct;

/*Struct to control material properties */
typedef struct Material
{
	float3 ambientColor; //KA
	float3 diffuseColor; //KD
	float3 specularColor;//KS
}Material;



/* Möller–Trumbore intersection algorithm - http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm */
int Intersect(const float3   V1,  // Triangle vertices
	const float3   V2,
	const float3   V3,
	const float3    O,  //Ray origin
	const float3    D,  //Ray direction
	float3*	 normal,
	float3*	 point_i,
	float*	 dist,
	float* out)
{

	float3 e1, e2;  //Edge1, Edge2
	float3 P, Q, T;
	float det, inv_det, u, v;
	float t, t2;

	//Find intersection and distance

	//Find vectors for two edges sharing V1
	e1 = V2 - V1;
	e2 = V3 - V1;
	//Begin calculating determinant - also used to calculate u parameter, this is used to calculate normal as well, so we calculate here now
	P = cross(D, e2);
	//if determinant is near zero, ray lies in plane of triangle
	det = dot(e1, P);
	//NOT CULLING
	if (det > -SMALL_NUM && det < SMALL_NUM) return 0;
	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = O - V1;

	//Calculate u parameter and test bound
	u = dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if (u < 0.f || u > 1.f) return 0;

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate V parameter and test bound
	v = dot(D, Q) * inv_det;
	//The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f) return 0;

	t2 = dot(e2, Q);
	t = t2 * inv_det;

	if (t > SMALL_NUM) { //ray intersection

		*normal = cross(e1, e2);
		float r = t2 * inv_det;			//if there was a intersection, compute distance!
		*point_i = (O)+(D)* r; // intersect point of ray and plane

		*dist = r;
		*out = t;

		return 1;
	}

	// No hit, no win
	return 0;


}

/* Here starts the raytracer*/
__kernel void Raytrace(__global float3* vertices, __global int4* faces, __global SceneGroupStruct* groups, __global Material* materials,
	__global SceneInformation* sceneInfo, __global uchar4* frame, __global Camera* camera, __global Light* light)
{
	int id = get_global_id(0);
	// grab XY coordinate of this instance
	int x = id % sceneInfo->width;
	int y = id / sceneInfo->height;


	/* Using the syntax frame[x][y] produces different behaviour on different platforms (doesn't work on NVIDIA GPUS)
	So use the XYZ to access the members of any vector types */

	/* build direction of the ray based on camera and the current pixel */
	float normalized_i = -((float)((float)x / (float)(sceneInfo->width) * (float)(sceneInfo->proportion_x)) - 0.5f);
	float normalized_j = -((float)((float)y / (float)(sceneInfo->height) * (float)(sceneInfo->proportion_y)) - 0.5f);
	float3 ray_dir = (float3)(camera->right * normalized_i) + (float3)(camera->up * normalized_j) + camera->dir;

	ray_dir = normalize(ray_dir);

	float distance = 1000000.0f; // high value for the first ray
	int face_i = -1; // index of the face that was hit, was -1 I don't now why
	int groupIndex = -1;
	float maxDistance = 1000000.0f; //max distance, work as a far view point
	float3 point_i; // intersection point
	float3 normal; // face normal
	float3 l_origin = camera->pos; // local copy of origin of rays (camera/eye)
	float intersectOutput;
	/* this is used to correct the offsets of the index inside the faces buffer, since they are using local indexes
		(related to the group to which they are associated) */
	int faceOffset = 0;
	for (unsigned int p = 0; p < sceneInfo->groupsSize; p++)
	{
		// for each face, look for intersections with the ray
		int facesEnd = groups[p].facesStart + groups[p].facesSize; // the index where the faces of this group ends
		for (unsigned int k = groups[p].facesStart; k < facesEnd; k++)
		{
			int result;
			float3 temp_point; // temporary intersection point
			float3 temp_normal;// temporary normal vector

			result = Intersect(vertices[faces[k].x + faceOffset], 
								vertices[faces[k].y + faceOffset], 
								vertices[faces[k].z + faceOffset],
								l_origin, ray_dir, &temp_normal, &temp_point, &distance, &intersectOutput);

			if (result > 0)
			{
				//some collision
				if (distance < maxDistance) // check if it's the closest to the camera so far
				{
					maxDistance = distance;
					face_i = k;
					point_i = temp_point;
					normal = temp_normal;
					groupIndex = p;
				}
			}
		}
		/* update offset for the next group */
		faceOffset += groups[p].facesSize;
	}
	// paint pixel
	if (face_i != -1)
	{

		// get direction vector of light based on the intersection point
		float3 L = light->pos - point_i;
		L = normalize(L);

		///* shot secondary ray directed to the light and see if we have a shadow */
		//l_origin = point_i;
		//bool shadow = false;
		//float3 temp; // info to be discarted
		//for (unsigned int p = 0; p < sceneInfo->facesSize; p++)
		//{
		//	if (p != face_i)
		//	{
		//		if (Intersect(vertices[faces[p].x], vertices[faces[p].y], vertices[faces[p].z], l_origin, L, &temp, &temp, &distance, &intersectOutput) > 0)
		//		{
		//			shadow = true;
		//			break;
		//		}
		//	}
		//}
		//if (shadow)// if there's a shadow, put black pixel
		//{
		//	frame[id].x = 0;
		//	frame[id].y = 0;
		//	frame[id].z = 0;
		//	frame[id].w = 255;
		//	return;
		//}

		// now that we have the face, calculate illumination
		float3 amount_color = (float3)(0.0f, 0.0f, 0.0f); //final amount of color that goes to each pixel

		normal = normalize(normal);

		//int indexMaterial = faces[face_i].w;

		//diffuse
		float dot_r = dot(normal, L);
		if (dot_r > 0)
		{
			float Kd = ((materials[groupIndex].diffuseColor.x
				+ materials[groupIndex].diffuseColor.y
				+ materials[groupIndex].diffuseColor.z) * 0.3333);
			float dif = dot_r * Kd;
			//put diffuse component
			amount_color += materials[groupIndex].diffuseColor * light->color * dif;
		}
		//specular
		//glm::vec3 R = glm::cross(2.0f * glm::dot(L,normal) * normal,L);
		float3 R = L - 2.0f * dot(L, normal) * normal;
		dot_r = dot(ray_dir, R);
		if (dot_r > 0)
		{
			float spec = pown(dot_r, 20.0f) * light->Ks;
			// put specular component
			amount_color += spec * light->color;
		}

		// build pixel
		float3 final_c;
		final_c.x = (amount_color.x) + (light->color.x * light->Ka); // put ambient
		final_c.y = (amount_color.y) + (light->color.y * light->Ka);
		final_c.z = (amount_color.z) + (light->color.z * light->Ka);

		if (final_c.x > 1.0f)
			final_c.x = 1.0f;
		if (final_c.y > 1.0f)
			final_c.y = 1.0f;
		if (final_c.z > 1.0f)
			final_c.z = 1.0f;

		frame[id].x = (final_c.x * 255.0f);
		frame[id].y = (final_c.y * 255.0f);
		frame[id].z = (final_c.z * 255.0f);
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