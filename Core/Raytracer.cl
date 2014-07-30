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

#define SMALL_NUM  0.00000001f // anything that avoids division overflow


/*----------FXAA quality defines-----------
-------------------------------------------
-----------------------------------------*/
#define FXAA_EDGE_THRESHOLD	1/8		//The minimum amount of local contrast required to apply algorithm. 1 / 3 – too little, 1 / 4 – low quality, 1 / 8 – high quality, 1 / 16 – overkill
#define FXAA_EDGE_THRESHOLD_MIN 1/16	//Trims the algorithm from processing darks. 1 / 32 – visible limit, 1 / 16 – high quality, 1 / 12 – upper limit(start of visible unfiltered edges)
#define FXAA_SUBPIX_TRIM_SCALE 0 //Toggle subpix filtering. 0 – turn off, 1 – turn on, 2 – turn on force full (ignore FXAA_SUBPIX_TRIM and CAP)
#define FXAA_SUBPIX_TRIM 1/4 //Controls removal of sub-pixel aliasing., 1/2 – low removal, 1/3 – medium removal, 1/4 – default removal, 1/8 – high removal, 0 – complete removal
#define FXAA_SUBPIX_CAP 3/4 //Insures fine detail is not completely removed., This partly overrides FXAA_SUBPIX_TRIM., 3/4 – default amount of filtering, 7/8 – high amount of filtering, 1 – no capping of filtering
#define FXAA_SEARCH_STEPS 1 //Controls the maximum number of search steps., Multiply by FXAA_SEARCH_ACCELERATION for filtering radius., 
#define FXAA_SEARCH_ACCELERATION 1	//How much to accelerate search using anisotropic filtering., 1 – no acceleration, 2 – skip by 2 pixels, 3 – skip by 3 pixels, 4 – skip by 4 pixels (hard upper limit)
#define FXAA_SEARCH_THRESHOLD 1/4 //Controls when to stop searching. 1/4 – seems best quality wise

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
	int verticesSize;
	int normalSize;
	int facesSize;
	int materiaslSize;
	float proportion_x;
	float proportion_y;
} SceneInformation;

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
	//*dist = length(T);
	//*point_i = (O)+(D) * (*dist);

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
__kernel void Raytrace(__global float3* vertices, __global float3* normals, __global int4* faces, __global Material* materials,
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
	float maxDistance = 1000000.0f; //max distance, work as a far view point
	float3 point_i; // intersection point
	float3 normal; // face normal
	float3 l_origin = camera->pos; // local copy of origin of rays (camera/eye)
	float intersectOutput;
	// for each face, look for intersections with the ray
	for (unsigned int k = 0; k < sceneInfo->facesSize; k++)
	{
		int result;
		float3 temp_point; // temporary intersection point
		float3 temp_normal;// temporary normal vector

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

		// get direction vector of light based on the intersection point
		float3 L = light->pos - point_i;
		L = normalize(L);

		/* shot secondary ray directed to the light and see if we have a shadow */
		l_origin = point_i;
		bool shadow = false;
		float3 temp; // info to be discarted
		for (unsigned int p = 0; p < sceneInfo->facesSize; p++)
		{
			if (p != face_i)
			{
				if (Intersect(vertices[faces[p].x], vertices[faces[p].y], vertices[faces[p].z], l_origin, L, &temp, &temp, &distance, &intersectOutput) > 0)
				{
					shadow = true;
					break;
				}
			}
		}
		if (shadow)// if there's a shadow, put black pixel
		{
			frame[id].x = 0;
			frame[id].y = 0;
			frame[id].z = 0;
			frame[id].w = 255;
			return;
		}

		// now that we have the face, calculate illumination
		float3 amount_color = (float3)(0.0f, 0.0f, 0.0f); //final amount of color that goes to each pixel

		normal = normalize(normal);

		int indexMaterial = faces[face_i].w;	// material is stored in the last component of the face vector

		//diffuse
		float dot_r = dot(normal, L);
		if (dot_r > 0)
		{
			float Kd = ((materials[indexMaterial].diffuseColor.x
				+ materials[indexMaterial].diffuseColor.y
				+ materials[indexMaterial].diffuseColor.z) / 3.0f);
			float dif = dot_r * Kd;
			//put diffuse component
			amount_color += materials[indexMaterial].diffuseColor * light->color * dif;
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


uchar FxaaLuma(uchar3 rgb) {
	return rgb.y * (0.587 / 0.299) + rgb.x;
}

__kernel void AntiAliasingFXAA(__global uchar4* screenInput, __global uchar4* screenOutput, __global int* width, __global int* height)
{
	int id = get_global_id(0);
	int x = id % *width;				//-----column in which is the pixel
	int y = id / *width;				//-----line in which is the pixel

	bool canUp = true;
	bool canDown = true;
	bool canWest = true;
	bool canEast = true;

	uchar3 rgbM = screenInput[id].xyz;
	//get current pixel and transform
	uchar lumaM = FxaaLuma(rgbM);

	uchar3 rgbW = rgbM;
	uchar3 rgbE = rgbM;
	uchar3 rgbN = rgbM;
	uchar3 rgbS = rgbM;
	uchar lumaW = lumaM;
	uchar lumaE = lumaM;
	uchar lumaN = lumaM;
	uchar lumaS = lumaM;

	//find the pixel of each coordinate and transform
	if (x > 0)
	{
		rgbW = screenInput[id - 1].xyz;
		lumaW = FxaaLuma(rgbN);
	}
	else
	{
		canWest = false;
	}
	if (x < *width - 1)
	{
		rgbE = screenInput[id + 1].xyz;
		lumaE = FxaaLuma(rgbE);
	}
	else
	{
		canEast = false;
	}
	if (y > 0)
	{
		rgbN = screenInput[id - *width].xyz;
		lumaN = FxaaLuma(rgbN);
	}
	else
	{
		canUp = false;
	}
	if (y < *height - 1)
	{
		rgbS = screenInput[id + *width].xyz;
		lumaS = FxaaLuma(rgbS);
	}
	else
	{
		canDown = false;
	}


	float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
	float range = rangeMax - rangeMin;
	if (range <
		max((float)FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD)) {
		screenOutput[id] = screenInput[id];
		return;	}	uchar lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
	float rangeL = abs(lumaL - lumaM);
	float blendL = max(0.0f,
		(rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
	blendL = min((float)FXAA_SUBPIX_CAP, blendL);	uchar3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;

	////////fazer condição pra cada direção!!!!
	uchar3 rgbNW = rgbM;
	uchar3 rgbNE = rgbM;
	uchar3 rgbSW = rgbM;
	uchar3 rgbSE = rgbM;
	uchar lumaNW = lumaM;
	uchar lumaNE = lumaM;
	uchar lumaSW = lumaM;
	uchar lumaSE = lumaM;

	if (canUp && canWest)
	{
		rgbNW = screenInput[id - *width - 1].xyz;
		lumaNW = FxaaLuma(rgbNW);
	}
	if (canUp && canEast)
	{
		rgbNE = screenInput[id - *width + 1].xyz;
		lumaNE = FxaaLuma(rgbNE);
	}
	if (canDown && canWest)
	{
		rgbSW = screenInput[id + *width - 1].xyz;
		lumaSW = FxaaLuma(rgbSW);
	}
	if (canDown && canEast)
	{
		rgbSE = screenInput[id + *width + 1].xyz;
		lumaSE = FxaaLuma(rgbSE);
	}

	rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
	rgbL = rgbL * (uchar3)(0.1111f, 0.1111f, 0.1111f);			//dont know if it's possible
	screenOutput[id].x = rgbL.x;
	screenOutput[id].y = rgbL.y;
	screenOutput[id].z = rgbL.z;
	screenOutput[id].w = 255;
	//this part cannot be done without texture!!	/*-------------------------------------------
	//edge test
	float edgeVert =
	abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
	abs((0.50 * lumaW) + (-1.0 * lumaM) + (0.50 * lumaE)) +
	abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
	float edgeHorz =
	abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
	abs((0.50 * lumaN) + (-1.0 * lumaM) + (0.50 * lumaS)) +
	abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
	bool horzSpan = edgeHorz >= edgeVert;	float3 posN;	float3 posP;	float lumaEndN;	float lumaEndP;	lumaN = lumaM;	float gradientN = FXAA_SEARCH_THRESHOLD;	if (horzSpan)	{	posN = rgbW;	posP = rgbE;	}	else	{	posN = rgbS;	posP = rgbM;	}
	bool doneN = false;
	bool doneP = false;

	//search for edges
	for (uint i = 0; i < FXAA_SEARCH_STEPS; i++) {
	#if FXAA_SEARCH_ACCELERATION == 1					//MUST BE 1!!
	if (!doneN) lumaEndN = FxaaLuma(posN.xyz);
	if (!doneP) lumaEndP = FxaaLuma(posP.xyz);
	#else
	if (!doneN) lumaEndN = FxaaLuma(
	FxaaTextureGrad(tex, posN.xy, offNP).xyz);
	if (!doneP) lumaEndP = FxaaLuma(
	FxaaTextureGrad(tex, posP.xy, offNP).xyz);
	#endif
	doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
	doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
	if (doneN && doneP) break;
	if (!doneN) posN -= offNP;
	if (!doneP) posP += offNP;
	}
	------------------------------------------------*/

}