#include "kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(
/**
 * @brief Contains the raytracing OpenCL code.
 * 
 * @return The code as a string
 */
string opencl_c_container() { return R( // ########################## begin of OpenCL C code ####################################################################

// Return wether x is in [y - range, y + range]
bool inrange(float x, float y, float range)
{
	return y - range < x && y + range > x;
}

// Emulates the vec as a float4 consisting of x, y, z, m
// Matrix multiplication from vec3 to vec4
float4 matmul34(const float3 vec, const float m, const float16 mat)
{
	return (float4) (
		vec.s0 * mat.s0 + vec.s1 * mat.s1 + vec.s2 * mat.s2 + m * mat.s3,
		vec.s0 * mat.s4 + vec.s1 * mat.s5 + vec.s2 * mat.s6 + m * mat.s7,
		vec.s0 * mat.s8 + vec.s1 * mat.s9 + vec.s2 * mat.sA + m * mat.sB,
		vec.s0 * mat.sC + vec.s1 * mat.sD + vec.s2 * mat.sE + m * mat.sF
	);
}

// Matrix multiplication from vec4 to vec4
float4 matmul44(const float4 vec, const float16 mat)
{
	return (float4) (
		vec.s0 * mat.s0 + vec.s1 * mat.s1 + vec.s2 * mat.s2 + vec.s3 * mat.s3,
		vec.s0 * mat.s4 + vec.s1 * mat.s5 + vec.s2 * mat.s6 + vec.s3 * mat.s7,
		vec.s0 * mat.s8 + vec.s1 * mat.s9 + vec.s2 * mat.sA + vec.s3 * mat.sB,
		vec.s0 * mat.sC + vec.s1 * mat.sD + vec.s2 * mat.sE + vec.s3 * mat.sF
	);
}

// Matrix multiplication from vec4 to vec4 with transposed mat
float4 matmul44T(const float4 vec, const float16 mat)
{
	return (float4) (
		vec.s0 * mat.s0 + vec.s1 * mat.s4  + vec.s2 * mat.s8 + vec.s3 * mat.sC,
		vec.s0 * mat.s1 + vec.s1 * mat.s5  + vec.s2 * mat.s9 + vec.s3 * mat.sD,
		vec.s0 * mat.s2 + vec.s1 * mat.s6  + vec.s2 * mat.sA + vec.s3 * mat.sE,
		vec.s0 * mat.s3 + vec.s1 * mat.s7  + vec.s2 * mat.sB + vec.s3 * mat.sF
	);
}

// Matrix multiplication from vec4 to vec3
float3 matmul43(const float4 vec, const float16 mat)
{
	float4 t = matmul44(vec, mat);
	// It's a direction vector
	if (t.w == 0.f) return (float3) (t.xyz);
	return (float3) (t.x / t.w, t.y / t.w, t.z / t.w);
}

// Matrix multiplication from vec4 to vec3 with transposed mat
float3 matmul43T(const float4 vec, const float16 mat)
{
	float4 t = matmul44T(vec, mat);
	// It's a direction vector
	if (t.w == 0.f) return (float3) (t.xyz);
	return (float3) (t.x / t.w, t.y / t.w, t.z / t.w);
}

)+R(

// Calculates distance of a point and an object.
float4 dist(const float4 first, const float8 second) {
	return (float4) (first.xyz - second.xyz, 0.f);
}

float rdotHC(const float4 first, const float4 second)
{
	return first.x * second.x + first.y * second.y + first.z * second.z;
}

float rlengthHC(const float4 vec)
{
	return sqrt(rdotHC(vec, vec));
}

// Scales color based on y value of raydirection (y).
float3 color(const float3 dir, global float* amb_col)
{
	float3 unit_dir = normalize(dir);
	// Standard larp
	float t = 0.5 * (unit_dir.y + 1.0);
	return (1.0f - t) * (float3)(1.0, 1.0, 1.0) + t * (float3)(amb_col[0], amb_col[1], amb_col[2]);
}

// Adds two intensities (e.g. color indices) logarithmically
float intensity_addition(const float left, const float right)
{
	if (left > 1.f || right > 1.f) return 1.f;
	return 1.f - (1.f - left) * (1.f - right);
}

// Adds two colors logarithmically
float4 color_addition(const float4 left, const float4 right)
{
	return (float4) (intensity_addition(left.x, right.x),
					 intensity_addition(left.y, right.y),
					 intensity_addition(left.z, right.z),
					 left.w);
}
float3 color_addition3(const float3 left, const float3 right)
{
	return (float3) (intensity_addition(left.x, right.x),
					 intensity_addition(left.y, right.y),
					 intensity_addition(left.z, right.z));
}

)+R(
// This function calculates the ray hit on an object and returns
// the resulting start and dir vectors packed into a float8.
float4 calc_rays(float3 start, float3 dir, float8 object, float16 mat, float16 invmat)
{
	float4 modStart = matmul34(start, 1.f, invmat);
	float4 modDir = matmul34(dir, 0.f, invmat);

	float t;
	float3 N;

	if (object.s5 == 0.f) {
		float4 sc = dist(modStart, object);
		float a = dot(modDir, modDir);
		float half_b = dot(sc, modDir);
		float c = dot(sc, sc) - object.s3 * object.s3;
		float discr = half_b * half_b - a * c;
		if (discr < 0.00001f) return (float4) (-1.0f, -1.0f, -1.0f, -1.0f);
		
		t = (-half_b - sqrt(discr)) / a;
		if (t < 0.00001f)
		{
			t = (-half_b + sqrt(discr)) / a;
			if (t < 0.00001f) return (float4) (-1.0f, -1.0f, -1.0f, -1.0f);
		}
		
		float4 mP = modStart + t * modDir;
		float4 mN = (float4) ((mP.x - object.x) / object.s3,
			(mP.y - object.y) / object.s3,
			(mP.z - object.z) / object.s3, 0.f);

		N = matmul43T(mN, invmat);

	} else if (object.s5 == 1.f) {

		float4 mN;
		if (object.s3 == 1.f) mN = (float4) (0.f, 1.f, 0.f, 0.f);
		else mN = (float4) (0.f, -1.f, 0.f, 0.f);

		float nd = dot(mN, modDir);
		if (nd < 0.00001f && nd > -0.00001f)
			return (float4) (-1.0f, -1.0f, -1.0f, -1.0f);
		
		float nos = -rdotHC(mN, modStart);
		t = -(nos / nd);
		if (t > 0.f) return (float4) (-1.0f, -1.0f, -1.0f, -1.0f);

		N = matmul43T(mN, invmat);
	}
	else return (float4) (-1.0f, -1.0f, -1.0f, -1.0f);
	
	return (float4) (N.xyz, t);
}
/*
void pushElement(int item, global int* stack, int* index)
{
	stack[++(*index)] = item;
}
int popElement(global int* stack, int* index)
{
	return stack[*index--];
}
void pushElementF(float item, global float* stack, int* index)
{
	stack[++(*index)] = item;
}
float popElementF(global float* stack, int* index)
{
	return stack[*index--];
}
void pushElementF4(float4 item, global float4* stack, int* index)
{
	stack[++(*index)] = item;
}
float4 popElementF4(global float4* stack, int* index)
{
	return stack[*index--];
}
char is_in(const int val, const int3 act)
{
	return act.x == val || act.y == val || act.z == val;
}

// Modelled after http://ceur-ws.org/Vol-1576/090.pdf and https://github.com/bstempelj/webgl-csg-raytracing/blob/master/shaders/raytracer.frag.js
)+R(
float8 find_closest(float3 start, float3 dir, global float8* objects,
	global float16* objectMats, global float16* objectInvMats, global int4* cmpInfo, float tree_height,
	global int* state_stack, global float* time_stack, global float4* prim_stack, uint n)
{
	const int NONE = -1;
	const int GotoLft = 1;
	const int GotoRgh = 2;
	const int Compute = 3;
	const int LoadLft = 4;
	const int LoadRgh = 5;
	const int SaveLft = 6;

	float t_min = 0.f;
	uint node = 0;	
	float4 l = (float4) (-1.f, -1.f, -1.f, -1.f);
	float4 r = (float4) (-1.f, -1.f, -1.f, -1.f);
	unsigned stateHead = -1, timeHead = -1, primHead = -1;
	pushElement(Compute, state_stack, &stateHead);
	int state = GotoLft;

	char gotoL = 0, gotoR = 0;

	while (true)
	{
		if (state == SaveLft)
		{
			t_min = popElementF(time_stack, &timeHead);
			// PushPrimitive(l)
			pushElementF4(l, prim_stack, &primHead);
			state = GotoRgh;
		}
		else if (state == GotoLft || state == GotoRgh)
		{

			if (state == GotoLft) node = cmpInfo[node].x;
			else node = cmpInfo[node].y;

			if (node >= 0) // Node is an operation
			{
				gotoL = 1;
				gotoR = 1;

				if (gotoL && cmpInfo[node].x < 0)
				{
					int i = cmpInfo[node].x;
					l = calc_rays(start, dir, objects[-i], objectMats[-i], objectInvMats[-i]);
					gotoL = 0;
				}
				if (gotoR && cmpInfo[node].y < 0)
				{
					int i = cmpInfo[node].y;
					r = calc_rays(start, dir, objects[-i], objectMats[-i], objectInvMats[-i]);
					gotoR = 0;
				}
				if (gotoL || gotoR)
				{
					if (gotoL)
					{
						//PushPrimitive(L(node), t_l)
						pushElementF4(l, prim_stack, &primHead);
						pushElement(LoadLft, state_stack, &stateHead);
					}
					else if (gotoR)
					{
						pushElementF4(r, prim_stack, &primHead);
						pushElement(LoadRgh, state_stack, &stateHead);
					}
					else
					{
						pushElementF(t_min, time_stack, &timeHead);
						pushElement(LoadLft, state_stack, &stateHead);
						pushElement(SaveLft, state_stack, &stateHead);
					}
					if (gotoL) state = GotoLft;
					else state = GotoRgh;
				}
				else state = Compute;
			}
			else // Node is a primitive
			{
				if (state == GotoLft) l = calc_rays(start, dir, objects[-node], objectMats[-node], objectInvMats[-node]);
				else r = calc_rays(start, dir, objects[-node], objectMats[-node], objectInvMats[-node]);
				
				state = Compute;
				node = cmpInfo[node].z;
			}
		}
		else if (state == LoadLft || state == LoadRgh || state == Compute)
		{
			const int NONE = -2;
			const int MISS = -1;
			const int RETR = 0;
			const int RETL = 1;
			const int RETLIFCLOSER = 2;
			const int RETRIFCLOSER = 3;
			const int LOOPL = 4;
			const int LOOPR = 5;
			const int LOOPLIFCLOSER = 6;
			const int LOOPRIFCLOSER = 7;
			const int FLIPR = 8;

			if (state == LoadLft) l = popElementF4(prim_stack, &primHead);
			else if (state == LoadRgh) r = popElementF4(prim_stack, &primHead);

			char stateL, stateR;
			// Classify
			if (l.x == l.y && l.y == l.z && l.z == l.w && l.w == -1.f) stateL = 0;
			else stateL = dot(as_float3(l), l.w * dir) < 0.f? -1 : 1;
			if (r.x == r.y && r.y == r.z && r.z == r.w && r.w == -1.f) stateR = 0;
			else stateR = dot(as_float3(r), r.w * dir) < 0.f? -1 : 1;

			// Table search
			int3 actions;
			switch(cmpInfo[node].w)
			{
			case 0: // Union
				if (stateL == 0 && stateR == 0) actions = (int3) (MISS, NONE, NONE);
				else if (stateL == 0) actions = (int3) (RETR, NONE, NONE);
				else if (stateR == 0) actions = (int3) (RETL, NONE, NONE);
				else if (stateL < 0 && stateR < 0) actions = (int3) (RETLIFCLOSER, RETRIFCLOSER, NONE);
				else if (stateL > 0 && stateR > 0) actions = (int3) (LOOPLIFCLOSER, LOOPRIFCLOSER, NONE);
				else if (stateL < 0 && stateR > 0) actions = (int3) (RETRIFCLOSER, LOOPL, NONE);
				else actions = (int3) (RETLIFCLOSER, LOOPR, NONE);
				break;
			case 1: // Intersection
				if (stateL == 0 || stateR == 0) actions = (int3) (MISS, NONE, NONE);
				else if (stateL < 0 && stateR < 0) actions = (int3) (LOOPLIFCLOSER, LOOPRIFCLOSER, NONE);
				else if (stateL > 0 && stateR > 0) actions = (int3) (RETLIFCLOSER, RETRIFCLOSER, NONE);
				else if (stateL < 0 && stateR > 0) actions = (int3) (RETLIFCLOSER, LOOPR, NONE);
				else actions = (int3) (RETRIFCLOSER, LOOPL, NONE);
				break;
			case 2: // Subtraction
				if (stateL == 0) actions = (int3) (MISS, NONE, NONE);
				else if (stateR == 0) actions = (int3) (RETL, NONE, NONE);
				else if (stateL < 0 && stateR < 0) actions = (int3) (RETLIFCLOSER, LOOPR, NONE);
				else if (stateL > 0 && stateR > 0) actions = (int3) (RETRIFCLOSER, FLIPR, LOOPL);
				else if (stateL < 0 && stateR > 0) actions = (int3) (LOOPLIFCLOSER, LOOPRIFCLOSER, NONE);
				else actions = (int3) (RETLIFCLOSER, RETRIFCLOSER, FLIPR);
				break;
			default: // What?
				actions = (int3) (NONE, NONE, NONE);
			}
			if (is_in(MISS, actions)) return (float8) (-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f);

			if (is_in(RETL, actions) || (is_in(RETLIFCLOSER, actions) && l.w <= r.w))
			{
				r = l;
				state = popElement(state_stack, &stateHead);
				node = cmpInfo[node].z;
			}
			if (is_in(RETR, actions) || (is_in(RETRIFCLOSER, actions) && r.w < l.w))
			{
				if (is_in(FLIPR, actions))
					l = (float4) (-r.x, -r.y, -r.z, r.w);
				else l = r;
				state = popElement(state_stack, &stateHead);
				node = cmpInfo[node].z;
			}
			else if (is_in(LOOPL, actions) || (is_in(LOOPLIFCLOSER, actions) && l.w <= r.w))
			{
				t_min = l.w;
				pushElementF4(r, prim_stack, &primHead);
				pushElement(LoadRgh, state_stack, &stateHead);
				state = GotoLft;
			}
			else if (is_in(LOOPR, actions) || (is_in(LOOPRIFCLOSER, actions) && r.w < l.w))
			{
				t_min = r.w;
				pushElementF4(l, prim_stack, &primHead);
				pushElement(LoadLft, state_stack, &stateHead);
				state = GotoRgh;
			}
			else
			{
				r.w = -1.f;
				state = popElement(state_stack, &stateHead);
			}
		}
		if (stateHead == -1 && node == 0) break;
	}
	if (t_min < 0.0001) return (float8) (-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f);
	float3 P = start + t_min * dir;
	return (float8) (P.x, P.y, P.z, 0.f, l.x, l.y, l.z, 0.f);
}*/

float rtAbs(float a)
{
	return a < 0.f? -a : a;
}
float3 refract(const float3 dir, const float3 n, float nonp)
{
	float cos_theta = min(dot(-dir, n), 1.f);
	float3 r_out_perp = nonp * (dir + cos_theta * n);
	float3 r_out_parallel = -sqrt(rtAbs(1.f - dot(r_out_perp, r_out_perp))) * n;
	return r_out_perp + r_out_parallel;
}

float8 find_closest(float3 start, float3 dir, global float8* objects,
	global float16* objectMats, global float16* objectInvMats, float objNum)
{
	float4 shortest = (float4) (0.f, 0.f, 0.f, 100000.f);
	uint ind = 0;
	for (uint i = 0; i < (int)objNum; i++)
	{
		float4 t = calc_rays(start, dir, objects[i], objectMats[i], objectInvMats[i]);
		if (t.x == t.y && t.y == t.z && t.z == t.w && t.w == -1.f) continue;
		if (rtAbs(t.w) < shortest.w)
		{
			shortest = t;
			ind = i;
		}
	}
	if (shortest.w == 100000.f) return (float8) (-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f);
	return (float8) (start + shortest.w * dir, 0.f, shortest.xyz, (float)ind);
}
bool light_reachable(float3 P, float8 light, global float8* objects,global float16* objectMats, global float16* objectInvMats, float objNum)
{
	float3 V = P - light.xyz;
	float8 t = find_closest(P, V, objects, objectMats, objectInvMats, objNum);
	return t.s3 == -1.f || length(t.xyz) > length(V);
}

)+R(

// Base ray calculation as to be called from the CPU.
kernel void ray_kernel(global float4* start1, global float4* dir1,
	global float4* start2, global float4* dir2,
	global float4* out, global float* ambient_data,
	global float8* objects, global float16* objectMats, global float16* objectInvMats, //global int4* cmpInfo,
	global float16* materials, global float8* lights) {
	const uint n = get_global_id(0);

	// We have an uninitialized vector, either because no reflection was found here or because
	// something went wrong - anyways, we don't need to do any calculations here.
	if (start1[n].x == 0. && start1[n].y == 0. && start1[n].z == 0. &&
		dir1[n].x == 0. && dir1[n].y == 0. && start1[n].z == 0.) return;

	bool last = start2 == NULL && dir2 == NULL;

	const float8 res = find_closest(as_float3(start1[n]), as_float3(dir1[n]), objects, objectMats, objectInvMats, ambient_data[6]);
	// We're looking at the sky and don't need further calculations
	if (res.s3 == -1.0f) {
		float3 c = color(dir1[n].xyz, ambient_data);
		out[n] = (float4) (c.xyz, start1[n].w);
		return;
	}
	else { // Reflection found! That unfortunately means further calculations
		float16 mat = materials[(int)objects[(int)res.s7].s4];

		const float3 I = as_float3(start1[n] + dir1[n]);
		const float3 N = (float3) (res.s456);
		const float3 P = (float3) (res.s012);
		const float3 REF = as_float3(dir1[n]) - 2.f * dot(as_float3(dir1[n]), N) * N;
		const float3 V = normalize(start1[n].xyz - P);

		// K_a * I_a
		float3 ambc = mat.s0 * (float3)(ambient_data[0], ambient_data[1], ambient_data[2]);

		float3 difc = (float3) (0.f, 0.f, 0.f);
		float3 spec = (float3) (0.f, 0.f, 0.f);
		for (uint li = 0; li < (int)ambient_data[7]; li++)
		{
			// Vector from light source to object point
			float3 l = normalize(lights[li].xyz - P);
			if (!light_reachable(P + N * 0.1f, lights[li], objects, objectMats, objectInvMats, ambient_data[6])) continue;
			// Dot product with normal
			float lambertian = max(dot(l, N), 0.f);
			float specular = 0.f;
			if (lambertian > 0.0001f)
			{
				// Perfectly reflected light ray
				float3 r = -l - 2 * dot(-l, N) * N;
				float specAngle = max(dot(r, V), 0.f);
				specular = pow(specAngle, mat.s6);
			}

			difc = color_addition3(difc, mat.s1 * lambertian * mat.s789);
			spec = color_addition3(spec, mat.s2 * specular * lights[li].s345);
		}

		out[n] = (float4) ( //ambc + difc + spec,
			intensity_addition(ambc.x, intensity_addition(difc.x, spec.x)),
			intensity_addition(ambc.y, intensity_addition(difc.y, spec.y)),
			intensity_addition(ambc.z, intensity_addition(difc.z, spec.z)),
			start1[n].w
		);
		if (!last)
		{
			// Reflected rays
			dir2[2 * n] = (float4) (REF.xyz, mat.s5);
			start2[2 * n] = (float4) (P.xyz, mat.s3);

			// Refracted rays
			float refr = dot(dir1[n].xyz, N) > 0.f? dir1[n].w / mat.s5 : mat.s5 / dir1[n].w;
			float cos_theta = min(dot(-dir1[n].xyz, n), 1.f);
			float sin_theta = sqrt(1.f - cos_theta * cos_theta);
			bool can_refract = refr * sin_theta <= 1.f;
			if (can_refract)
			{
				start2[2 * n + 1] = (float4) (P.xyz, mat.s4);
				dir2[2 * n + 1] = (float4) (refract(dir1[n].xyz, N, refr), mat.s5);
			}
		}
	}
}

)+R(

bool null(const float4 op)
{
	return op.x == -1.f && op.y == 0.f && op.z == 0.f && op.w == 0.f;
}

// Creates the color of the incoming ray.
// cur_level here represents the reflection colors, next_level the
// ray color.
// How you can imagine this working is that the kernels work their way up from
// last reflected/refracted rays to the original, screen rays in reverse order
// From what ray_kernel did.
kernel void color_kernel(global float4* cur_level, global float4* next_level) {
	const uint n = get_global_id(0);

	// Calculates the indices of the reflected and the refracted ray in the cur_level array.
	const int first = 2 * n;
	const int second = 2 * n + 1;

	float4 add_color;

	// If both rays (or one of them) didn't get calculated (for whatever reason),
	// We don't have to do any color addition and can simply skip that.
	if (null(cur_level[first]) && null(cur_level[second]))
		return;
	// Only refraction
	else if (null(cur_level[first]))
		add_color = cur_level[second].w * cur_level[second];
	// Only reflection
	else if (null(cur_level[second]))
		add_color = cur_level[first].w * cur_level[first];
	// Both
	else
		add_color = color_addition(cur_level[first].w * cur_level[first], cur_level[second].w * cur_level[second]);
	
	next_level[n] = color_addition(next_level[n], add_color);
	//next_level[n] = cur_level[first];
}

);} // ############################################################### end of OpenCL C code #####################################################################
