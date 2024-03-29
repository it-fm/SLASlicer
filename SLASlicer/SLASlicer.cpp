#include <iostream>
#include <array>

using namespace std;

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include <windows.h>

#include "ctpl_stl.h"

#include "STLSlicer.h"

static bool slice(int threadID, STLSlicer &slicer, vector<cv::Mat*> slices, int z, fp_t zPosition);

int main()
{
	// hard code bean bvalues for now (assuming 0.05mm slice thickness)
	// TODO: get from a JSON config file (or command line?)
	array<size_t, 3> volSize = { 1440, 2560, 800 };														// bean
	Point3D buildVol = { (fp_t)68, (fp_t)120, (fp_t)160 };												// bean (mm)
	Point3D voxelSize = { (fp_t)0.05, (fp_t)0.05, (fp_t)0.05 };											// bean (mm) (or should X,Y be buildVol / volSize)?
	Point3D origin = { (fp_t)(volSize[0] / 2.0 - 0.5), (fp_t)(volSize[1] / 2.0 - 0.5), (fp_t)(-0.5) };	// fractional index

	STLGeometry geometry;
	cin >> geometry;

	// hard code for the test model
	// TODO: also encode transformations in the JSON config file?
	geometry.Scale((fp_t)0.5, (fp_t)0.5, (fp_t)0.5);

	// need to create after all transformations are complete
	STLSlicer slicer(&geometry, volSize, buildVol, origin, voxelSize);

	int threadCount = std::thread::hardware_concurrency();
	ctpl::thread_pool pool(threadCount);

	vector<cv::Mat*> slices;
	for (int i = 0; i < threadCount; i++)
		slices.push_back(new cv::Mat((int)volSize[1], (int)volSize[0], CV_8U));

	// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
	std::vector<std::future<bool>> results(volSize[2]);
	for (int z = 0; z < volSize[2]; z++)
	{
		fp_t zPosition = (z - origin[2]) * voxelSize[2];
		results[z] = pool.push(std::ref(slice), slicer, slices, z, zPosition);
	}
	for (int z = 0; z < volSize[2]; z++)
	{
		if (!results[z].get())
		{
			cout << "final slice: " << z << endl;

			break;
		}
	}

	// done slicing
	// TODO: do whatever clean-up necessary (eg zip the directory)
	return 0;
}

static bool slice(int threadID, STLSlicer &slicer, vector<cv::Mat*> slices, int z, fp_t zPosition)
{
	if (slicer.Slice(slices[threadID], zPosition))
	{
		stringstream filename;
		// TODO: hardcode file name for now -- add to config file?
		filename << "slices\\slice" << z << ".png";

		try
		{
			// TODO: optionally use morphological open/close operations to remove fine structures
			// TODO: is imwrite() thread safe?
			imwrite(filename.str().c_str(), *(slices[threadID]));
		}
		catch (runtime_error& ex)
		{
			cerr << "Exception converting image to PNG format: " << ex.what() << endl;
			return false;
		}
	}
	return true;
}
