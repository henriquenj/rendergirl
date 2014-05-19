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
} SceneInformation;

/*Struct to control material properties */
typedef struct Material
{
	double3 ambientColor; //KA
	double3 diffuseColor; //KD
	double3 specularColor;//KS
}Material;



/* Intersect function test collision with triangles only, based on a function provided by Rossana Baptista Queiroz */
int Intersect(double* dist, double3* origin, double3* dir, double3* point, int indexFace, double3* normal,
	__global int4* faces, __global double3* vertices)
{
	// get position of the three vertices of the triangle
	double3 tri[3];

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

	double3 u, v, n; //triangle vectors
	double3 w0, w; //ray vectors
	double r, a, b; // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = tri[1] - tri[0];
	v = tri[2] - tri[0];
	*normal = cross(u, v);
	double3 zero = (double3)(0.0, 0.0, 0.0);
	if (all(*normal == zero))
		return -1; // triangle is degenerate, not deal with this case

	w0 = (*origin) - tri[0];
	a = -dot(*normal, w0);
	b = dot(*normal, *dir);
	if (fabs(b) < SMALL_NUM) // ray is parallel to triangle plane
	{
		if (a == 0)				// ray lies in triangle plane
			return 1;
		else return 0;	// ray disjoint from plane
	}


	// get intersection point of ray with triangle plane
	r = a / b;
	if (r < 0.0) // ray goes away from triangle
		return 0; // => no intersect
	// for a segment, also test if (r > 1.0) => no intersect

	*point = (*origin) + (*dir) * r; // intersect point of ray and plane

	*dist = r;

	// is I inside T?
	double    uu, uv, vv, wu, wv, D;
	uu = dot(u, u);
	uv = dot(u, v);
	vv = dot(v, v);
	w = (*point) - tri[0];
	wu = dot(w, u);
	wv = dot(w, v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	double s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)        // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                      // I is in T

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
	double normalized_i = (double)((double)x / (double)(sceneInfo->width)) - 0.5;
	double normalized_j = (double)((double)y / (double)(sceneInfo->height)) - 0.5;
	double3 ray_dir = (double3)(camera->right * normalized_i) + (double3)(camera->up * normalized_j) + camera->dir;

	ray_dir = normalize(ray_dir);

	double distance = 1000000.0; // high value for the first ray
	int face_i = -1; // index of the face that was hit
	double maxDistance = 1000000.0; //max distance, work as a far view point
	double3 point_i; // intersection point
	double3 normal; // face normal
	double3 l_origin = camera->pos; // local copy of origin of rays (camera/eye)
	// for each face, look for intersections with the ray
	for (unsigned int k = 0; k < sceneInfo->facesSize; k++)
	{
		int result;
		double3 temp_point; // temporary intersection point
		double3 temp_normal;// temporary normal vector

		result = Intersect(&distance, &l_origin, &ray_dir, &temp_point, k, &temp_normal, faces, vertices);

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

		/* shot secondary ray directed to the light and see if we have a shadow */
		//double3 rayToLight = point_i - light->pos;
		//rayToLight = normalize(rayToLight);
		//l_origin = light->pos;
		//int temp; // info to be discarted
		//for (unsigned int p = 0; p < sceneInfo->facesSize; p++)
		//{
		//	if (p != face_i)
		//	{
		//		if (Intersect(&distance, &l_origin, &rayToLight, &temp, p, &temp, faces, vertices) > 0)
		//		{
		//			frame[id].x = 0;
		//			frame[id].y = 0;
		//			frame[id].z = 0;
		//			frame[id].w = 255;
		//			return;
		//		}
		//	}
		//}

		// now that we have the face, calculate illumination
		double3 amount_color = (double3)(0.0, 0.0, 0.0); //final amount of color that goes to each pixel

		// get direction vector of light based on the intersection point
		double3 L = light->pos - point_i;
		L = normalize(L);
		normal = normalize(normal);

		int indexMaterial = faces[face_i].w;// material is stored in the last component of the face vector

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