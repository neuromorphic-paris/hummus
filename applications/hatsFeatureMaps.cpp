/*
 * hatsNetwork.cpp
 * Adonis_c - clock-driven spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 09/10/2018
 *
 * Information: Spiking neural network running with histograms of averaged time surfaces converted into spikes.
 */

#include <iostream> 

#include "../source/network.hpp"
#include "../source/qtDisplay.hpp"
#include "../source/testOutputLogger.hpp"
#include "../source/stdp.hpp"

int main(int argc, char** argv)
{
    //  ----- INITIALISING THE NETWORK -----
//	adonis_c::QtDisplay qtDisplay;
	adonis_c::TestOutputLogger testOutputLogger("hatsFeatureMaps.bin");
	adonis_c::Network network({&testOutputLogger});
	
    //  ----- NETWORK PARAMETERS -----
	
    // IDs for each layer (order is important)
    int layer0 = 0;
    int layer1 = 1;
    int layer2 = 2;
	
	int gridWidth = 42;
	int gridHeight = 35;
	int rfSize = 7;
	
	float decayCurrent = 10;
	float decayPotential = 20;
	float refractoryPeriod = 3;
	bool burstingActivity = false;
	float eligibilityDecay = 20;
	
	//  ----- INITIALISING THE LEARNING RULE -----
	adonis_c::Stdp stdp(layer0, layer1);
	
	//  ----- CREATING THE NETWORK -----
	network.add2dLayer(layer0, rfSize, gridWidth, gridHeight, &stdp, 3, -1, false, decayCurrent, decayPotential, refractoryPeriod, burstingActivity, eligibilityDecay);
	network.add2dLayer(layer1, rfSize, gridWidth, gridHeight, &stdp, 1, 1, false, decayCurrent, decayPotential, refractoryPeriod, burstingActivity, eligibilityDecay);
	network.addLayer(layer2, nullptr, 2, 1, 1, decayCurrent, decayPotential, 1000, burstingActivity, eligibilityDecay);

	network.convolution(network.getLayers()[layer0], network.getLayers()[layer1], false, 1./15, false, 0);
	network.allToAll(network.getLayers()[layer1], network.getLayers()[layer2], false, 1./10, false, 0);
	
	//  ----- READING TRAINING DATA FROM FILE -----
	adonis_c::DataParser dataParser;
    auto trainingData = dataParser.readTrainingData("../../data/hats/feature_maps/nCars_10samplePerc_1rep.txt");
	
    //  ----- INJECTING TRAINING SPIKES -----
	network.injectSpikeFromData(&trainingData);
	
	//  ----- READING TEST DATA FROM FILE -----
	auto testingData = dataParser.readTestData(&network, "../../data/hats/feature_maps/nCars_1samplePerc_1rep.txt");
	
	//  ----- INJECTING TEST SPIKES -----
	network.injectSpikeFromData(&testingData);

//	// ----- ADDING LABELS
//	auto labels = dataParser.readLabels("" , "../../data/hats/feature_maps/nCars_1samplePerc_1repLabel.txt");

//	network.addLabels(&labels);
	
//    //  ----- DISPLAY SETTINGS -----
//  	qtDisplay.useHardwareAcceleration(true);
//  	qtDisplay.setTimeWindow(5000);
//  	qtDisplay.trackLayer(2);
//  	qtDisplay.trackInputSublayer(0);
//  	qtDisplay.trackNeuron(network.getNeurons().back().getNeuronID());
	
    //  ----- RUNNING THE NETWORK -----
    float runtime = testingData.back().timestamp+1000;
	float timestep = 0.5;
	
    network.run(runtime, timestep);
	
    //  ----- EXITING APPLICATION -----
    return 0;
}
