/*
 * unsupervisedNetwork.cpp
 * Hummus - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 14/01/2019
 *
 * Information: Example of a spiking neural network that can learn one dimensional patterns.
 */

#include <iostream>

#include "../source/core.hpp"
#include "../source/dataParser.hpp"
#include "../source/GUI/qtDisplay.hpp"
#include "../source/addOns/spikeLogger.hpp"
#include "../source/addOns/potentialLogger.hpp"
#include "../source/addOns/myelinPlasticityLogger.hpp"
#include "../source/learningRules/myelinPlasticity.hpp"
#include "../source/neurons/inputNeuron.hpp"
#include "../source/neurons/LIF.hpp"

int main(int argc, char** argv) {
    //  ----- READING TRAINING DATA FROM FILE -----
	hummus::DataParser dataParser;
	
	auto trainingData = dataParser.readData("../../data/1D_patterns/oneD_10neurons_4patterns_.txt");
	
    //  ----- INITIALISING THE NETWORK -----
	hummus::QtDisplay qtDisplay;
	hummus::SpikeLogger spikeLog("10neurons_4patterns_unsupervised_spikeLog.bin");
	hummus::MyelinPlasticityLogger myelinPlasticityLog("10neurons_4patterns_unsupervised_learningLog.bin");
    hummus::Network network({&spikeLog, &myelinPlasticityLog}, &qtDisplay);
    
    //  ----- NETWORK PARAMETERS -----
	float resetCurrent = 10;
	float potentialDecay = 20;
    int inputNeurons = 10;
    int layer1Neurons = 4;
	
	float eligibilityDecay = 20;
	
	bool wta = true;
	bool burst = false;
	bool homeostasis = false;
	
	//  ----- INITIALISING THE LEARNING RULE -----
	hummus::MyelinPlasticity myelinPlasticity(1, 1, 1, 1);
	
    //  ----- CREATING THE NETWORK -----
    network.addLayer<hummus::InputNeuron>(inputNeurons, 1, 1, {});
    network.addLayer<hummus::LIF>(layer1Neurons, 1, 1, {&myelinPlasticity}, true, homeostasis, resetCurrent, potentialDecay, 3, wta, burst, eligibilityDecay);
	
	//  ----- CONNECTING THE NETWORK -----
	network.allToAll(network.getLayers()[0], network.getLayers()[1], 0.2, 0.1, 5, 3);
    
    //  ----- DISPLAY SETTINGS -----
	qtDisplay.useHardwareAcceleration(true);
	qtDisplay.setTimeWindow(5000);
	qtDisplay.trackNeuron(11);

	network.turnOffLearning(80000);

    //  ----- RUNNING THE NETWORK -----
    network.run(&trainingData, 0.1);
	
    //  ----- EXITING APPLICATION -----
    return 0;
}
