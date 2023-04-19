#include <memory>
#include <cmath>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencl.hpp>

#include <object.hpp>
#include <baseobject.hpp>
#include <complexobject.hpp>
#include <transformedobject.hpp>
#include <fulltransobject.hpp>
#include <interpreter.hpp>

/// @brief This function searches the object tree and writes found information in order of those objects into the given memory objects.
/// @param objects Data about the base objects
/// @param objectMats Object matrices
/// @param invMats Object inverse matrices
/// @param cmpInfo Information about complex combinations
/// @param obj The top object in the tree
/// @param objNum Put in a variable set to the index of the first uninitialized object in objects
void search(Memory<cl_float8>& objects, Memory<cl_float16>& objectMats,
	Memory<cl_float16>& invMats, /*Memory<cl_int4>& cmpInfo, */std::shared_ptr<Raytracing::Object> obj, unsigned& objNum)
{
	if (obj->type() == Raytracing::ObjectType::Fulltransform)
	{
		// Because we know we are dealing with a fulltransform object and not a normal object now
		auto fto = std::dynamic_pointer_cast<Raytracing::Fulltransform>(obj);
		auto bso = std::dynamic_pointer_cast<Raytracing::BaseObject>(fto->obj);
		// Type information of the base object
		const float t = bso->bt == Raytracing::BaseTypes::Sphere ? 0.f : 1.f;
		objects[objNum] = {
			// Oh no, we can't downcast in a {}-initializer O.o
			static_cast<float>(bso->pos.x()),
			static_cast<float>(bso->pos.y()),
			static_cast<float>(bso->pos.z()),
			static_cast<float>(bso->rd),
			static_cast<float>(bso->mat_id),
			t, 0.f, 0.f
		};
		auto m = fto->matrix;
		objectMats[objNum] = {
			static_cast<float>(m.mat[0][0]), static_cast<float>(m.mat[0][1]), static_cast<float>(m.mat[0][2]), static_cast<float>(m.mat[0][3]),
			static_cast<float>(m.mat[1][0]), static_cast<float>(m.mat[1][1]), static_cast<float>(m.mat[1][2]), static_cast<float>(m.mat[1][3]),
			static_cast<float>(m.mat[2][0]), static_cast<float>(m.mat[2][1]), static_cast<float>(m.mat[2][2]), static_cast<float>(m.mat[2][3]),
			static_cast<float>(m.mat[3][0]), static_cast<float>(m.mat[3][1]), static_cast<float>(m.mat[3][2]), static_cast<float>(m.mat[3][3]),
		};
		m = fto->invmatrix;
		invMats[objNum] = {
			static_cast<float>(m.mat[0][0]), static_cast<float>(m.mat[0][1]), static_cast<float>(m.mat[0][2]), static_cast<float>(m.mat[0][3]),
			static_cast<float>(m.mat[1][0]), static_cast<float>(m.mat[1][1]), static_cast<float>(m.mat[1][2]), static_cast<float>(m.mat[1][3]),
			static_cast<float>(m.mat[2][0]), static_cast<float>(m.mat[2][1]), static_cast<float>(m.mat[2][2]), static_cast<float>(m.mat[2][3]),
			static_cast<float>(m.mat[3][0]), static_cast<float>(m.mat[3][1]), static_cast<float>(m.mat[3][2]), static_cast<float>(m.mat[3][3]),
		};
		objNum++;
		return;
	}
	else if (obj->type() == Raytracing::ObjectType::Complex)
	{
		/*// Copy the current index
		int n = cmpNum;
		// Put operation information into the cmpInfo
		cmpInfo[n].s[3] = Utility::to_underlying(std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->operation);
		// Link to the next objects in the tree
		cmpInfo[n].s[0] = search(objects, objectMats, invMats, cmpInfo, std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->left, objNum, ++cmpNum);
		cmpInfo[n].s[1] = search(objects, objectMats, invMats, cmpInfo, std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->right, objNum, ++cmpNum);
		// If those objects are complex themselves, we need to doubly-link them
		if (cmpInfo[n].s[0] >= 0)
			cmpInfo[cmpInfo[n].s[0]].s[2] = n;
		if (cmpInfo[n].s[1] >= 0)
			cmpInfo[cmpInfo[n].s[1]].s[2] = n;
		return cmpNum;*/
		search(objects, objectMats, invMats, std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->left, objNum);
		search(objects, objectMats, invMats, std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->right, objNum);
	}
	else if (obj->type() == Raytracing::ObjectType::Base)
	{
		auto bso = std::dynamic_pointer_cast<Raytracing::BaseObject>(obj);
		// Type information of the base object
		const float t = bso->bt == Raytracing::BaseTypes::Sphere ? 0.f : 1.f;
		objects[objNum++] = {
			static_cast<float>(bso->pos.x()),
			static_cast<float>(bso->pos.y()),
			static_cast<float>(bso->pos.z()),
			static_cast<float>(bso->rd),
			static_cast<float>(bso->mat_id),
			t, 0.f, 0.f
		};
		return;
	}
}

int main(int argc, char* argv[]) {
	std::string filename;
	if (argc >= 2) filename = argv[1];
	else filename = "input.rti";
	Raytracing::Interpreter inp;
	try
	{
		inp.interpretFile(filename);
	}
	catch (Utility::Exception e)
	{
		Utility::printException(e);
		std::cin.get();
		return -1;
	}
	Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device

	const ulong N = inp.variables["width"] * inp.variables["height"]; // size of vectors

	{
		unsigned mu = sizeof(cl_float8) + inp.materials.size() * sizeof(cl_float16) +
			7 * sizeof(float) + inp.base_objs * (sizeof(cl_float8) + 2 * sizeof(cl_float16)) + inp.cmpOps * sizeof(cl_int4);
		for (unsigned i = 0; i < inp.variables["raydepth"]; i++)
			mu += 3 * N * pow(2, i) * sizeof(cl_float4);

		print_info("Total expected memory usage of program upon initialization: " + std::to_string(mu) + " bytes ("
			+ std::to_string(mu / 1024) + "kB, " + std::to_string(mu / 1024 / 1024) + "mB)");
		print_info("Due to executed code, the actual memory usage might be higher! This is dependent on your machine and OpenCL C compiler.");
	}

	// A base object (which is for now the only one handled) only needs three values for its position
	// and one value for its radius/direction; additionally one for the material. Additional
	// values are reserved for transformations/complex stuff in the future.
	Memory<cl_float8> objects(device, inp.base_objs, 1U, true, true, cl_float8 {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f});
	Memory<cl_float16> objectMats(device, inp.base_objs, 1U, true, true, cl_float16 {1.f, 0.f, 0.f, 0.f,
																					0.f, 1.f, 0.f, 0.f,
																					0.f, 0.f, 1.f, 0.f,
																					0.f, 0.f, 0.f, 1.f});
	Memory<cl_float16> objectInvMats(device, inp.base_objs, 1U, true, true, cl_float16 {1.f, 0.f, 0.f, 0.f,
																						0.f, 1.f, 0.f, 0.f,
																						0.f, 0.f, 1.f, 0.f,
																						0.f, 0.f, 0.f, 1.f});
	//Memory<cl_int4> complexInfo(device, inp.cmpOps + 1, 1U, true, true, cl_int4 {-1, 0, 0, 0});
	Memory<cl_float16> materials(device, inp.materials.size(), 1U, true, true, cl_float16 {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f});
	Memory<float> ambient_data(device, 8);
	Memory<cl_float8> lights(device, inp.lightSources.size(), 1U, true, true, cl_float8 {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f});

	std::vector<Memory<cl_float4>> starts(inp.variables["raydepth"]);
	std::vector<Memory<cl_float4>> dirs(inp.variables["raydepth"]);
	std::vector<Memory<cl_float4>> colors(inp.variables["raydepth"]);
	for (unsigned i = 0; i < starts.size(); i++)
	{
		starts[i] = Memory<cl_float4>(device, N * pow(2, i), 1U, true, true, cl_float4 {0.f, 0.f, 0.f, 0.f});
		dirs[i] = Memory<cl_float4>(device, N * pow(2, i), 1U, true, true, cl_float4 {0.f, 0.f, 0.f, 0.f});
		colors[i] = Memory<cl_float4>(device, N * pow(2, i), 1U, true, true, cl_float4 {-1.f, 0.f, 0.f, 0.f});
		starts[i].write_to_device();
		dirs[i].write_to_device();
		colors[i].write_to_device();
	}
	for(unsigned i = 0; i < starts[0].length(); i++) {
		starts[0][i] = { (float)inp.rays[i]->start.x(), (float)inp.rays[i]->start.y(), (float)inp.rays[i]->start.z(), 1.f };
		dirs[0][i] = { (float)inp.rays[i]->dir.x(), (float)inp.rays[i]->dir.y(), (float)inp.rays[i]->dir.z(), 1.f };
	}
	starts[0].write_to_device();
	dirs[0].write_to_device();
	print_info("Set up device memory.");

	ambient_data[0] = inp.variables["ambient_r"];
	ambient_data[1] = inp.variables["ambient_g"];
	ambient_data[2] = inp.variables["ambient_b"];
	ambient_data[3] = inp.variables["ambient_int_r"];
	ambient_data[4] = inp.variables["ambient_int_g"];
	ambient_data[5] = inp.variables["ambient_int_b"];
	ambient_data[6] = static_cast<float>(inp.base_objs);
	ambient_data[7] = static_cast<float>(inp.lightSources.size());

	unsigned objNum {0};
	search(objects, objectMats, objectInvMats,/* complexInfo, */inp.topObject, objNum);

	for (const auto& i : inp.materials)
	{
		materials[i.second->mat_id] = {
			static_cast<float>(i.second->ambref),
			static_cast<float>(i.second->diffref),
			static_cast<float>(i.second->specref),
			static_cast<float>(i.second->rflec),
			static_cast<float>(i.second->rfrac),
			static_cast<float>(i.second->rfracind),
			static_cast<float>(i.second->shiny),
			static_cast<float>(i.second->color.x()),
			static_cast<float>(i.second->color.y()),
			static_cast<float>(i.second->color.z()), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
		};
	}
	unsigned lindex {0};
	for (const auto& i : inp.lightSources)
	{
		lights[lindex++] = {
			static_cast<float>(i.second->pos.x()),
			static_cast<float>(i.second->pos.y()),
			static_cast<float>(i.second->pos.z()),
			static_cast<float>(i.second->color.x()),
			static_cast<float>(i.second->color.y()),
			static_cast<float>(i.second->color.z()), 0.f, 0.f
		};
	}
	print_info("Initialized device memory...");
	ambient_data.write_to_device();
	objects.write_to_device();
	objectMats.write_to_device();
	objectInvMats.write_to_device();
	//complexInfo.write_to_device();
	materials.write_to_device();
	lights.write_to_device();

	print_info("Beginning raytracing...");
	for (unsigned i = 0; i < inp.variables["raydepth"] - 1; i++)
	{
		Kernel ray_kernel(device, starts[i].length(), "ray_kernel",
			starts[i], dirs[i], starts[i + 1], dirs[i + 1], colors[i],
			ambient_data, objects, objectMats, objectInvMats, /*complexInfo, */materials, lights); // kernel that runs on the device

		ray_kernel.run(); // run ray_kernel on the device
	}
	// The last rays in the reflection hierarchy don't create further rays, so they're passed a nullpointer.
	{
		unsigned i = inp.variables["raydepth"] - 1;
		Kernel ray_kernel(device, starts[i].length(),
			"ray_kernel", starts[i], dirs[i], NULL, NULL, colors[i],
			ambient_data, objects, objectMats, objectInvMats, /*complexInfo, */materials, lights);
		ray_kernel.run();
	}

	print_info("Done with raytracing, computing colors...");

	for (unsigned i = inp.variables["raydepth"] - 1; i > 0; i--)
	{
		Kernel color_kernel(device, colors[i - 1].length(), "color_kernel", colors[i], colors[i - 1]);
		color_kernel.run();
	}
	colors[0].read_from_device();

	print_info("Done with color computation.");

	std::string win = "Raytracing Output";
	cv::namedWindow(win, cv::WINDOW_AUTOSIZE);
	auto ar = Utility::openclMemToArray(colors[0]);
	cv::Mat matrix((int)inp.variables["height"], (int)inp.variables["width"], CV_8UC3, ar.array);
	cv::imshow(win, matrix);

	cv::waitKey(0);
	cv::destroyAllWindows();

	print_info("Press any key to exit...");
	wait();

	return 0;
}
