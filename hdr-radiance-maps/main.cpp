#include "DebevecHDRRadianceMapsImpl.h"
#include <iostream>

int main(int argc, char** argv)
{
//	   Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 2);
//	   std::cout << "Here is the matrix A:\n" << A << std::endl;
//	   Eigen::VectorXf b = Eigen::VectorXf::Random(3);
//	   std::cout << "Here is the right hand side b:\n" << b << std::endl;
//	   std::cout << "The least-squares solution is:\n"
//	        << A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b) << std::endl;

	if (argc < 2)
	{
		std::cerr << "Error: | Application requires a parameter file!" << std::endl;
		return (-1);
	}
	const std::string paramFile(argv[1]);

	DebevecHDRRadianceMapsImpl hdrMap;
	bool initialized = hdrMap.init(paramFile);
	if (!initialized)
	{
		return (-1);
	}
	hdrMap.run();
	return (0);
}

