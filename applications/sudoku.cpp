/*
 * sudoku.cpp
 * Baal - clock-driven spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 17/04/2018
 *
 * Information: spiking neural network trained to solve a 4x4 sudoku.
 *
 * Connectivity scheme:
 * 1- each domain is connected to the same domain on other layers for lateral inhibition
 * 2- horizontal inhibition within each layer
 * 3- vertical lateral inhibition within each layer
 * 4- lateral inhibition within each subgrid of each layer
 *
 *	  ----------- 			 -----------
 *	 |2 |  |  |1 |			|2 |4 |3 |1 |
 *	 |  |3 |  |  |			|1 |3 |4 |2 |
 *	 |  |  |1 |  |			|4 |2 |1 |3 |
 *	 |3 |  |  |4 |			|3 |1 |2 |4 |
 *	  -----------			 -----------
 *		SUDOKU                SOLUTION
 */

#include <iostream>

#include "../source/dataParser.hpp"
#include "../source/network.hpp"
#include "../source/display.hpp"
#include "../source/logger.hpp"

int main(int argc, char** argv)
{
//  ----- READING DATA FROM FILE -----
    baal::DataParser dataParser;
    auto data = dataParser.readData("../data/sudoku/sudokuRandomSpikesFixed.txt");
    
//  ----- INITIALISING THE NETWORK -----
	baal::Network network;

//  ----- NETWORK PARAMETERS -----    
    float runtime = 100;
	float timestep = 1;
	
//  ----- CREATING THE LAYERS -----
    network.addReceptiveFields(16, 0, baal::learningMode::noLearning, 8);
    network.addReceptiveFields(16, 1, baal::learningMode::noLearning, 8);
    network.addReceptiveFields(16, 2, baal::learningMode::noLearning, 8);
    network.addReceptiveFields(16, 3, baal::learningMode::noLearning, 8);
    network.addReceptiveFields(16, 4, baal::learningMode::noLearning, 8);

//  ----- CONNECTING THE LAYERS -----
    // lateral inhibition on domains with the same coordinates
    for (auto& receptiveField1: network.getNeuronPopulations())
    {
        if (receptiveField.layerID != 0)
        {
            for (auto& receptiveField2: network.getNeuronPopulations())
            {
                if (receptiveField1.rfID == receptiveField2.rfID)
                {
                    network.allToallConnectivity(&receptiveField1.rfNeurons, &receptiveField2.rfNeurons, false, 0, false, 0)
                }
            }
        }
    }
    

//    for (auto i=0; i<(numberOfLayers-1)*std::pow(sudokuWidth,2); i+=std::pow(sudokuWidth,2))
//    {
//		// horizontal connections
//		int x = 0;
//		for (auto j=i; j<i+std::pow(sudokuWidth,2); j++)
//		{
//			if (j % sudokuWidth == 0 && j != i)
//			{
//				x++;
//			}

//			for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//			{
//				if (network.getNeuronPopulations()[k][0].getX() == x && j != k)
//				{
//					network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//				}
//			}
//		}

//		// vertical connections
//		int	y = 0;
//		for (auto j=i; j<i+std::pow(sudokuWidth,2); j++)
//		{
//			y++;
//			if (j % 4 == 0)
//			{
//				y=0;
//			}

//			for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//			{
//				if (network.getNeuronPopulations()[k][0].getY() == y && j != k)
//				{
//					network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//				}
//			}
//		}

////		// grid connections
//		for (auto j=i; j<i+std::pow(sudokuWidth,2); j++)
//		{
//			if (network.getNeuronPopulations()[j][0].getX() >= 0 && network.getNeuronPopulations()[j][0].getX() <= 1)
//			{
//				if (network.getNeuronPopulations()[j][0].getY() >= 0 && network.getNeuronPopulations()[j][0].getY() <= 1)
//				{
//					for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//					{
//						if (network.getNeuronPopulations()[k][0].getX() >= 0 && network.getNeuronPopulations()[k][0].getX() <= 1 && network.getNeuronPopulations()[k][0].getY() >= 0 && network.getNeuronPopulations()[k][0].getY() <= 1 && j != k)
//						{
//							network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//						}
//					}
//				}
//				else if (network.getNeuronPopulations()[j][0].getY() >= 2 && network.getNeuronPopulations()[j][0].getY() <= 3)
//				{
//					for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//					{
//						if (network.getNeuronPopulations()[k][0].getX() >= 0 && network.getNeuronPopulations()[k][0].getX() <= 1 && network.getNeuronPopulations()[k][0].getY() >= 2 && network.getNeuronPopulations()[k][0].getY() <= 3 && j != k)
//						{
//							network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//						}
//					}
//				}
//			}

//			else if (network.getNeuronPopulations()[j][0].getX() >= 2 && network.getNeuronPopulations()[j][0].getX() <= 3)
//			{
//				if (network.getNeuronPopulations()[j][0].getY() >= 0 && network.getNeuronPopulations()[j][0].getY() <= 1)
//				{
//					for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//					{
//						if (network.getNeuronPopulations()[k][0].getX() >= 2 && network.getNeuronPopulations()[k][0].getX() <= 3 && network.getNeuronPopulations()[k][0].getY() >= 0 && network.getNeuronPopulations()[k][0].getY() <= 1 && j != k)
//						{
//							network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//						}
//					}
//				}

//				else if (network.getNeuronPopulations()[j][0].getY() >= 2 && network.getNeuronPopulations()[j][0].getY() <= 3)
//				{
//					for (auto k=i; k<i+std::pow(sudokuWidth,2); k++)
//					{
//				        if (network.getNeuronPopulations()[k][0].getX() >= 2 && network.getNeuronPopulations()[k][0].getX() <= 3 && network.getNeuronPopulations()[k][0].getY() >= 2 && network.getNeuronPopulations()[k][0].getY() <= 3 && j != k)
//					    {
//						    network.allToallConnectivity(&network.getNeuronPopulations()[j], &network.getNeuronPopulations()[k], false, inhibitionWeight, false, 0, false);
//					    }
//					}
//				}
//			}
//        }
//    }

//    struct sudoku
//    {
//        int X;
//        int Y;
//        int layerID;
//    };
//    std::vector<sudoku> filledValues;

//    filledValues.push_back(sudoku{0,0,2});
//    filledValues.push_back(sudoku{0,3,1});
//    filledValues.push_back(sudoku{1,1,3});
//    filledValues.push_back(sudoku{2,2,1});
//    filledValues.push_back(sudoku{3,0,3});
//    filledValues.push_back(sudoku{3,3,4});

//    // input layer towards digit layers
//    for (auto i=std::pow(sudokuWidth,2)*(numberOfLayers-1); i<network.getNeuronPopulations().size(); i++)
//    {
//        for (auto j=0; j<std::pow(sudokuWidth,2)*(numberOfLayers-1); j++)
//        {
//            if (network.getNeuronPopulations()[i][0].getX() == network.getNeuronPopulations()[j][0].getX() && network.getNeuronPopulations()[i][0].getY() == network.getNeuronPopulations()[j][0].getY())
//            {
//                for (auto val: filledValues)
//                {
//                    if (network.getNeuronPopulations()[j][0].getX() == val.X && network.getNeuronPopulations()[j][0].getY() == val.Y && network.getNeuronPopulations()[j][0].getLayerID() == val.layerID)
//                    {
//                        network.allToallConnectivity(&network.getNeuronPopulations()[i], &network.getNeuronPopulations()[j], true, filledWeight, false, 0, false);
//                    }
//                }
//            }
//        }
//    }

////  ----- INJECTING SPIKES -----
//	for (auto i=0; i<data.size(); i++)
//	{
//		int nID = data[i].neuronID;
//		for (auto j=std::pow(sudokuWidth,2)*(numberOfLayers-1); j<network.getNeuronPopulations().size(); j++)
//		{
//			auto spikes = std::find_if(network.getNeuronPopulations()[j].begin(), network.getNeuronPopulations()[j].end(), [nID](const baal::Neuron& n){return n.getNeuronID() == nID;});
//			if (spikes != std::end(network.getNeuronPopulations()[j]))
//			{
//				network.injectSpike(network.getNeuronPopulations()[j][std::distance(network.getNeuronPopulations()[j].begin(), spikes)].prepareInitialSpike(data[i].timestamp));
//			}
//		}
//    }
  
//  ----- RUNNING THE NETWORK -----
    network.run(runtime, timestep);

//  ----- EXITING APPLICATION -----
    return 0;
}