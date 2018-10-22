/*
 * testNetwork.cpp
 * Adonis_c - clock-driven spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 31/05/2018
 *
 * Information: Example of a basic spiking neural network.
 */

#include <string>
#include <iostream>

#include "../source/json.hpp"
#include "../source/network.hpp"
#include "../source/qtDisplay.hpp"
#include "../source/dataParser.hpp"
#include "../source/spikeLogger.hpp"
#include "../source/learningLogger.hpp"
#include "../source/stdp.hpp"
#include "../source/myelinPlasticity.hpp"
#include "../source/myelinPlasticityReinforcement.hpp"

using json = nlohmann::json;

int main(int argc, char** argv)
{
//	std::ifstream input;
//	std::string filename;
//	if (argc == 2)
//	{
//		filename = argv[1];
//	}
//	else if (argc == 1)
//	{
//		std::cout << "enter a file name: " << std::endl;
//		std::cin >> filename;
//	}
//	input.open(filename.c_str());
//	if (input.fail())
//	{
//		throw std::logic_error(filename.append(" could not be opened"));
//	}
//	
//	std::cout << "yes" << std::endl;
//    //  ----- EXITING APPLICATION -----
    return 0;
}
