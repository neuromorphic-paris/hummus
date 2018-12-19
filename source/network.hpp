/* 
 * network.hpp
 * Adonis_c - clock-driven spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 11/12/2018
 *
 * Information: The Network class acts as a spike manager.
 */

#pragma once

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <deque>

#include "neuron.hpp"
#include "dataParser.hpp"
#include "learningRuleHandler.hpp"
#include "standardNetworkAddOn.hpp"
#include "mainThreadNetworkAddOn.hpp"

namespace adonis_c
{
	struct receptiveField
	{
		std::vector<std::size_t> neurons;
		int16_t                  row;
		int16_t                  col;
	};
	
	struct sublayer
	{
		std::vector<receptiveField> receptiveFields;
		int16_t                     ID;
	};
	
	struct layer
	{
		std::vector<sublayer> sublayers;
		int16_t               ID;
		int                   width;
		int                   height;
	};
	
    class Network
    {
    public:
		
		// ----- CONSTRUCTOR AND DESTRUCTOR ------
        Network(std::vector<StandardNetworkAddOn*> _stdAddOns = {}, MainThreadNetworkAddOn* _thAddOn = nullptr) :
            stdAddOns(_stdAddOns),
			thAddOn(_thAddOn),
			learningStatus(true),
			learningOffSignal(-1),
			maxDelay(0)
		{}
		
		Network(MainThreadNetworkAddOn* _thAddOn) : Network({}, _thAddOn)
		{}
		
		// ----- NEURON CREATION METHODS -----
		
		// add neurons
		void addLayer(std::vector<LearningRuleHandler*> _learningRuleHandler={}, int neuronNumber=1, int rfNumber=1, int _sublayerNumber=1, bool homeostasis=false, float _decayCurrent=10, float _decayPotential=20, int _refractoryPeriod=3, bool _wta=false, bool _burstingActivity=false, float _eligibilityDecay=100, float decayHomeostasis=10, float homeostasisBeta=1, float _threshold = -50, float  _restingPotential=-70, float _resetPotential=-70, float _inputResistance=50e9, float _externalCurrent=100, int16_t _rfID=0)
        {
        	unsigned long shift = 0;
			
        	int16_t layerID = 0;
        	if (!layers.empty())
        	{
        		for (auto& l: layers)
        		{
        			for (auto& s: l.sublayers)
        			{
						for (auto& r: s.receptiveFields)
						{
							shift += r.neurons.size();
						}
					}
				}
				layerID = layers.back().ID+1;
			}
			
			// building a layer of one dimensional sublayers with no receptiveFields
			int counter = 0;
			std::vector<sublayer> subTemp;
        	for (int16_t i=0; i<_sublayerNumber; i++)
        	{
        		std::vector<receptiveField> rfTemp;
        		for (int16_t j=0; j<rfNumber; j++)
        		{
					std::vector<std::size_t> neuronTemp;
					for (auto k=0+shift; k<neuronNumber+shift; k++)
					{
						neurons.emplace_back(k+counter, j, 0, i, layerID, _decayCurrent, _decayPotential, _refractoryPeriod, _burstingActivity, _eligibilityDecay, _threshold, _restingPotential, _resetPotential, _inputResistance, _externalCurrent,-1,-1,_learningRuleHandler, homeostasis, decayHomeostasis, homeostasisBeta, _wta);
						
						neuronTemp.emplace_back(neurons.size()-1);
					}
					rfTemp.emplace_back(receptiveField{neuronTemp, j, 0});
				}
				subTemp.emplace_back(sublayer{rfTemp, i});
				counter += neuronNumber;
			}
			layers.emplace_back(layer{subTemp, layerID, -1, -1});
        }
		
		// add a one dimnetional layer of neurons that are labelled according to the provided labels
		void addDecisionMakingLayer(std::string trainingLabelFilename, std::vector<LearningRuleHandler*> _learningRuleHandler={}, int _refractoryPeriod=1000, bool homeostasis=false, float _decayCurrent=10, float _decayPotential=20, bool _wta=true, bool _burstingActivity=false, float _eligibilityDecay=100, float decayHomeostasis=10, float homeostasisBeta=1, float _threshold = -50, float  _restingPotential=-70, float _resetPotential=-70, float _inputResistance=50e9, float _externalCurrent=100)
		{
			DataParser dataParser;
			trainingLabels = dataParser.readLabels(trainingLabelFilename);
			
			// find number of classes
			std::vector<std::string> uniqueLabels;
			for (auto& label: trainingLabels)
			{
				if (std::find(uniqueLabels.begin(), uniqueLabels.end(), label.name) == uniqueLabels.end())
				{
					uniqueLabels.emplace_back(label.name);
				}
			}
			
			// add decision-making neurons
			unsigned long shift = 0;
			int16_t layerID = 0;
        	if (!layers.empty())
        	{
        		for (auto& l: layers)
        		{
        			for (auto& s: l.sublayers)
        			{
						for (auto& r: s.receptiveFields)
						{
							shift += r.neurons.size();
						}
					}
				}
				layerID = layers.back().ID+1;
			}

			std::vector<std::size_t> neuronTemp;
			for (auto k=0+shift; k<static_cast<int>(uniqueLabels.size())+shift; k++)
			{
				neurons.emplace_back(k, 0, 0, 0, layerID, _decayCurrent, _decayPotential, _refractoryPeriod, _burstingActivity, _eligibilityDecay, _threshold, _restingPotential, _resetPotential, _inputResistance, _externalCurrent,-1,-1,_learningRuleHandler, homeostasis, decayHomeostasis, homeostasisBeta, _wta, uniqueLabels[k-shift]);
				
				neuronTemp.emplace_back(neurons.size()-1);
			}
			layers.emplace_back(layer{{sublayer{{receptiveField{neuronTemp, 0, 0}}, 0}}, layerID, -1, -1});
		}
		
		// adds a 2 dimentional grid of neurons
		void add2dLayer(int windowSize, int gridW, int gridH, std::vector<LearningRuleHandler*> _learningRuleHandler={}, int _sublayerNumber=1, int _numberOfNeurons=-1, bool overlap=false, bool homeostasis=false, float _decayCurrent=10, float _decayPotential=20, int _refractoryPeriod=3, bool _wta=false, bool _burstingActivity=false, float _eligibilityDecay=100, float decayHomeostasis=10, float homeostasisBeta=1, float _threshold = -50, float  _restingPotential=-70, float _resetPotential=-70, float _inputResistance=50e9, float _externalCurrent=100)
		{
			// error handling
			if (windowSize <= 0 || windowSize > gridW || windowSize > gridH)
			{
				throw std::logic_error("the selected window size is not valid");
			}

			if (_numberOfNeurons != -1 && _numberOfNeurons <= 0)
			{
				throw std::logic_error("the number of neurons selected is wrong");
			}
			
			int overlapSize = 0;
			if (overlap)
			{
				if (windowSize > 1)
				{
					overlapSize = windowSize-1;
				}
				else if (windowSize == 1)
				{
					throw std::logic_error("For a window size equal to 1, consider using a layer with contiguous receptive fields by setting the overlap to false");
				}
			}
			else
			{
				double dW_check = gridW / static_cast<double>(windowSize);
				double dH_check = gridH / static_cast<double>(windowSize);
				
				int iW_check = dW_check;
				int iH_check = dH_check;

				if (dW_check != iW_check || dH_check != iH_check)
				{
					throw std::logic_error("With contiguous receptive fields, the width and height of the grid need to be divisible by the receptive field size");
				}
			}
			
			unsigned long shift = 0;
			int16_t layerID = 0;
			if (!layers.empty())
			{
				for (auto& l: layers)
				{
					for (auto& s: l.sublayers)
					{
						for (auto& r: s.receptiveFields)
						{
							shift += r.neurons.size();
						}
					}
				}
				layerID = layers.back().ID+1;
			}
			
			int counter = 0;
			std::vector<sublayer> subTemp;
			for (int16_t i=0; i<_sublayerNumber; i++)
			{
				int x = 0;
				int y = 0;
				
				int col = 0;
				int row = 0;
				
				int rowShift = 0;
				int colShift = 0;
				
				int16_t rfCol = 0;
				int16_t rfRow = 0;
				unsigned long neuronCounter = shift;
				
				std::vector<std::size_t> neuronTemp;
				std::vector<receptiveField> rfTemp;
				while (true)
				{
					if (x == gridW-1 && y != gridH-1 && col == 0 && row == 0)
					{
						rfCol = 0;
						rfRow += 1;
						colShift = 0;
						rowShift += windowSize-overlapSize;
					}
					
					x = col+colShift;
					y = row+rowShift;
					
					if (_numberOfNeurons == -1)
					{
						neurons.emplace_back(neuronCounter+counter, rfRow, rfCol, i, layerID, _decayCurrent, _decayPotential, _refractoryPeriod, _burstingActivity, _eligibilityDecay, _threshold, _restingPotential, _resetPotential, _inputResistance, _externalCurrent, x, y, _learningRuleHandler, homeostasis, decayHomeostasis, homeostasisBeta, _wta);
					
						neuronCounter += 1;
					
						neuronTemp.emplace_back(neurons.size()-1);
					}
					
					col += 1;
					if (col == windowSize && row != windowSize-1)
					{
						col = 0;
						row += 1;
					}
					else if (col == windowSize && row == windowSize-1)
					{
						col = 0;
						row = 0;
						colShift += windowSize-overlapSize;
						if (_numberOfNeurons > 0)
						{
							for (auto j = 0; j < _numberOfNeurons; j++)
							{
								neurons.emplace_back(neuronCounter+counter, rfRow, rfCol, i, layerID, _decayCurrent, _decayPotential, _refractoryPeriod, _burstingActivity, _eligibilityDecay, _threshold, _restingPotential, _resetPotential, _inputResistance, _externalCurrent, -1, -1, _learningRuleHandler, homeostasis, decayHomeostasis, homeostasisBeta, _wta);
								
								neuronCounter += 1;
								
								neuronTemp.emplace_back(neurons.size()-1);
							}
						}
						rfTemp.emplace_back(receptiveField{neuronTemp, rfRow, rfCol});
						rfCol += 1;
						neuronTemp.clear();
					}
					
					if (x == gridW-1 && y == gridH-1)
					{
						break;
					}
					
					if (x > gridW-1 || y > gridH-1)
					{
						throw std::logic_error("the window and the grid do not match. recheck the size parameters");
					}
				}
				subTemp.emplace_back(sublayer{rfTemp, i});
				counter += neuronCounter;
			}
			layers.emplace_back(layer{subTemp, layerID, gridW, gridH});
		}
		
		// ----- LAYER CONNECTION METHODS -----
		
		// all to all connections (for everything including sublayers and receptive fields)
    	void allToAll(layer presynapticLayer, layer postsynapticLayer, float _weightMean=1, int _weightstdev=0, int _delayMean=0, int _delaystdev=0, int probability=100, bool redundantConnections=true)
    	{
    		maxDelay = std::max(maxDelay, _delayMean);
			
    		for (auto& preSub: presynapticLayer.sublayers)
    		{
    			for (auto& preRF: preSub.receptiveFields)
    			{
					for (auto& pre: preRF.neurons)
					{
						for (auto& postSub: postsynapticLayer.sublayers)
						{
							for (auto& postRF: postSub.receptiveFields)
    						{
    							for (auto& post: postRF.neurons)
								{
									std::random_device device;
									std::mt19937 randomEngine(device());
									std::normal_distribution<> delayRandom(_delayMean, _delaystdev);
									std::normal_distribution<> weightRandom(_weightMean, _weightstdev);
									int sign = _weightMean<0?-1:_weightMean>=0;
									neurons[pre].addAxon(&neurons[post], sign*std::abs(weightRandom(randomEngine)), std::abs(delayRandom(randomEngine)), probability, redundantConnections);
								}
							}
						}
					}
				}
			}
		}
		
		// interconnecting a layer with soft winner-takes-all axons, using negative weights
		void lateralInhibition(layer l, float _weightMean=-1, float _weightstdev=0, int probability=100, bool redundantConnections=true)
		{
			if (_weightMean != 0)
			{
				if (_weightMean > 0)
				{
					std::cout << "lateral inhibition axons must have negative weights. The input weight was automatically converted to its negative counterpart" << std::endl;
				}
				
				for (auto& preSub: l.sublayers)
				{
					for (auto& preRF: preSub.receptiveFields)
					{
						for (auto& pre: preRF.neurons)
						{
							for (auto& postSub: l.sublayers)
							{
								for (auto& postRF: postSub.receptiveFields)
								{
									for (auto& post: postRF.neurons)
									{
										if (pre != post)
										{
											std::random_device device;
											std::mt19937 randomEngine(device());
											std::normal_distribution<> weightRandom(_weightMean, _weightstdev);
											neurons[pre].addAxon(&neurons[post], -1*std::abs(weightRandom(randomEngine)), 0, probability, redundantConnections);
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				throw std::logic_error("lateral inhibition axons cannot have a null weight");
			}
		}
		
		// connecting two layers according to their receptive fields
		void convolution(layer presynapticLayer, layer postsynapticLayer, float _weightMean=1, float _weightstdev=0, int _delayMean=0, float _delaystdev=0, int probability=100, bool redundantConnections=true)
		{
			// restrict to layers of the same size
			if (presynapticLayer.width != postsynapticLayer.width || presynapticLayer.height != postsynapticLayer.height)
			{
				throw std::logic_error("Convoluting two layers requires them to be the same size");
			}
			
			maxDelay = std::max(maxDelay, _delayMean);
			
			for (auto& preSub: presynapticLayer.sublayers)
    		{
    			for (auto& preRF: preSub.receptiveFields)
    			{
    				for (auto& postSub: postsynapticLayer.sublayers)
					{
						for (auto& postRF: postSub.receptiveFields)
						{
							if (preRF.row == postRF.row && preRF.col == postRF.col)
							{
								for (auto& pre: preRF.neurons)
								{
									for (auto& post: postRF.neurons)
									{
										std::random_device device;
										std::mt19937 randomEngine(device());
										std::normal_distribution<> delayRandom(_delayMean, _delaystdev);
										std::normal_distribution<> weightRandom(_weightMean, _weightstdev);
										int sign = _weightMean<0?-1:_weightMean>=0;
										neurons[pre].addAxon(&neurons[post], sign*std::abs(weightRandom(randomEngine)), std::abs(delayRandom(randomEngine)), probability, redundantConnections);
									}
								}
							}
						}
					}
				}
			}
		}
		
		// subsampling connection of receptive fields
		void pooling(layer presynapticLayer, layer postsynapticLayer, float _weightMean=1, int _weightstdev=0, int _delayMean=0, int _delaystdev=0, int probability=100, bool redundantConnections=true)
		{
			auto preMaxRows = presynapticLayer.sublayers[0].receptiveFields.back().row+1;
			auto preMaxColumns = presynapticLayer.sublayers[0].receptiveFields.back().col+1;
			
			auto postMaxRows = postsynapticLayer.sublayers[0].receptiveFields.back().row+1;
			auto postMaxColumns = postsynapticLayer.sublayers[0].receptiveFields.back().col+1;
			
			float fRow_check = preMaxRows / static_cast<float>(postMaxRows);
			float fCol_check = preMaxColumns / static_cast<float>(postMaxColumns);
			
			int rowPoolingFactor = fRow_check;
			int colPoolingFactor = fCol_check;
			
			if (rowPoolingFactor != fRow_check || colPoolingFactor != fCol_check)
			{
				throw std::logic_error("the number of receptive fields in each layer is not proportional. The pooling factor cannot be calculated");
			}
			
			maxDelay = std::max(maxDelay, _delayMean);
			
			for (auto& preSub: presynapticLayer.sublayers)
			{
				for (auto& postSub: postsynapticLayer.sublayers)
				{
					// each presynaptic sublayer connects to the same postsynaptic sublayer
					if (preSub.ID == postSub.ID)
					{
						int rowShift = 0;
						int colShift = 0;
						for (auto& postRf: postSub.receptiveFields)
						{
							for (auto& preRf: preSub.receptiveFields)
							{
								if ( preRf.row >= 0+rowShift && preRf.row < rowPoolingFactor+rowShift && preRf.col >= 0+colShift && preRf.col < colPoolingFactor+colShift)
								{
									for (auto& pre: preRf.neurons)
									{
										for (auto& post: postRf.neurons)
										{
											std::random_device device;
											std::mt19937 randomEngine(device());
											std::normal_distribution<> delayRandom(_delayMean, _delaystdev);
											std::normal_distribution<> weightRandom(_weightMean, _weightstdev);
											int sign = _weightMean<0?-1:_weightMean>=0;
											neurons[pre].addAxon(&neurons[post], sign*std::abs(weightRandom(randomEngine)), std::abs(delayRandom(randomEngine)), probability, redundantConnections);
										}
									}
								}
							}
							colShift += colPoolingFactor;
							if (postRf.col == postMaxColumns-1)
							{
								colShift = 0;
								rowShift += rowPoolingFactor;
							}
						}
					}
				}
			}
		}

		// ----- PUBLIC NETWORK METHODS -----
		
		// add spike to the network
		void injectSpike(spike s)
        {
            initialSpikes.push_back(s);
        }

		// adding spikes generated by the network
        void injectGeneratedSpike(spike s)
        {
            generatedSpikes.insert(
                std::upper_bound(generatedSpikes.begin(), generatedSpikes.end(), s, [](spike one, spike two){return one.timestamp < two.timestamp;}),
                s);
        }
        
		// add spikes from file to the network
		void injectSpikeFromData(std::vector<input>* data)
		{
			// error handling
			if (neurons.empty())
			{
				throw std::logic_error("add neurons before injecting spikes");
			}
			
			if ((*data)[1].x == -1 && (*data)[1].y == -1)
			{
				for (auto& event: *data)
				{
					for (auto& l: layers[0].sublayers)
					{
						if (event.sublayerID == l.ID)
						{
							for (auto& r: l.receptiveFields)
							{
								for (auto& n: r.neurons)
								{
									if (neurons[n].getNeuronID() == event.neuronID)
									{
										injectSpike(neurons[n].prepareInitialSpike(event.timestamp));
									}
								}
							}
						}
						else if (event.sublayerID == -1)
						{
							for (auto& r: l.receptiveFields)
							{
								for (auto& n: r.neurons)
								{
									if (neurons[n].getNeuronID() == event.neuronID)
									{
										injectSpike(neurons[n].prepareInitialSpike(event.timestamp));
									}
								}
							}
						}
					}
				}
    		}
    		else
    		{
				for (auto& event: *data)
				{
					for (auto& l: layers[0].sublayers)
					{
						if (event.sublayerID == l.ID)
						{
							for (auto& r: l.receptiveFields)
							{
								for (auto& n: r.neurons)
								{
									if (neurons[n].getX() == event.x && neurons[n].getY() == event.y)
									{
										injectSpike(neurons[n].prepareInitialSpike(event.timestamp));
										break;
									}
								}
							}
						}
						else if (event.sublayerID == -1)
						{
							for (auto& r: l.receptiveFields)
							{
								for (auto& n: r.neurons)
								{
									if (neurons[n].getX() == event.x && neurons[n].getY() == event.y)
									{
										injectSpike(neurons[n].prepareInitialSpike(event.timestamp));
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		
		// turn off all learning rules (for cross-validation or test data)
		void turnOffLearning(double timestamp)
		{
			learningOffSignal = timestamp;
		}
		
		// clock-based running through the network
        void run(double _timestep, double _runtime)
        {
			globalLearningRuleMonitor();
			
			for (auto addon: stdAddOns)
			{
				addon->onStart(this);
			}
			
        	std::mutex sync;
        	if (thAddOn)
			{
				sync.lock();
			}
			
			std::thread spikeManager([&]
			{
				sync.lock();
				sync.unlock();
				
				std::cout << "Running the network..." << std::endl;
				std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
				
				if (!neurons.empty())
				{
					for (double i=0; i<_runtime; i+=_timestep)
					{
						if (!trainingLabels.empty())
						{
							if (trainingLabels.front().onset <= i)
							{
								currentLabel = trainingLabels.front().name;
								trainingLabels.pop_front();
							}
						}
					
						if (learningOffSignal != -1)
						{
							if (learningStatus==true && i >= learningOffSignal)
							{
								std::cout << "learning turned off at t=" << i << std::endl;
								learningStatus = false;
							}
						}
						
						for (auto& n: neurons)
						{
							update(&n, i, _timestep);
						}
					}
				}
				else
				{
					throw std::runtime_error("add neurons to the network before running it");
				}

				std::cout << "Done." << std::endl;

				std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
				std::cout << "it took " << elapsed_seconds.count() << "s to run." << std::endl;
				
				for (auto addon: stdAddOns)
				{
					addon->onCompleted(this);
				}
			});
			
			if (thAddOn)
			{
				thAddOn->begin(this, &sync);
			}
			
			spikeManager.join();
		}
		
		// running through the network and segregating between a training phase and a classification phase
        void run(float _timestep, std::vector<input>* trainingData, std::vector<input>* testData=nullptr, int shift=20)
        {
			globalLearningRuleMonitor();
			
			for (auto addon: stdAddOns)
			{
				addon->onStart(this);
			}
			
        	std::mutex sync;
        	if (thAddOn)
			{
				sync.lock();
			}
			
			std::thread spikeManager([&]
			{
				sync.lock();
				sync.unlock();
				
				// importing training data and running the network through the data
				train(_timestep, trainingData, shift);
				
				// importing test data and running the network through the data
				if (testData)
				{
					predict(_timestep, testData, shift);
				}
				
				for (auto addon: stdAddOns)
				{
					addon->onCompleted(this);
				}
			});
			
			if (thAddOn)
			{
				thAddOn->begin(this, &sync);
			}
			spikeManager.join();
		}
		
		// reset the network back to the initial conditions without changing the network build
		void reset()
		{
			initialSpikes.clear();
			generatedSpikes.clear();
			learningStatus = true;
			learningOffSignal = -1;
		}
		
		// ----- SETTERS AND GETTERS -----
		std::vector<Neuron>& getNeurons()
		{
			return neurons;
		}
		
		std::vector<layer>& getLayers()
		{
			return layers;
		}
		
		std::vector<StandardNetworkAddOn*>& getStandardAddOns()
		{
			return stdAddOns;
		}
		
		MainThreadNetworkAddOn* getMainThreadAddOn()
		{
			return thAddOn;
		}
		
		std::deque<spike>& getGeneratedSpikes()
        {
            return generatedSpikes;
        }
		
		bool getLearningStatus() const
        {
            return learningStatus;
        }
		
		std::string getCurrentLabel() const
		{
			return currentLabel;
		}
		
    protected:
    
		// -----PROTECTED NETWORK METHODS -----
		
		// importing training data and running the network through the data
		void train(double timestep, std::vector<input>* trainingData, int shift)
		{
			injectSpikeFromData(trainingData);
			
			std::cout << "Training the network..." << std::endl;
			std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
			if (!neurons.empty())
			{
				for (double i=0; i<trainingData->back().timestamp+maxDelay+shift; i+= timestep)
				{
					if (!trainingLabels.empty())
					{
						if (trainingLabels.front().onset <= i)
						{
							currentLabel = trainingLabels.front().name;
							trainingLabels.pop_front();
						}
					}
					
					if (learningOffSignal != -1)
					{
						if (learningStatus==true && i >= learningOffSignal)
						{
							learningStatus = false;
							std::cout << "learning turned off at t=" << i << std::endl;
						}
					}
				
					for (auto& n: neurons)
					{
						update(&n, i, timestep);
					}
				}
			}
			else
			{
				throw std::runtime_error("add neurons to the network before running it");
			}
			std::cout << "Done." << std::endl;
			std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
			std::cout << "it took " << elapsed_seconds.count() << "s for the training phase." << std::endl;
		}
		
		// importing test data and running it through the network for classification
		void predict(double timestep, std::vector<input>* testData, int shift)
		{
			learningStatus = false;
			initialSpikes.clear();
			generatedSpikes.clear();
			for (auto& n: neurons)
			{
				n.resetNeuron();
			}
			
			injectSpikeFromData(testData);
			
			std::cout << "Running prediction based on a trained network..." << std::endl;
			if (!neurons.empty())
			{
				for (double i=0; i<testData->back().timestamp+maxDelay+shift; i+= timestep)
				{
					for (auto& n: neurons)
					{
						update(&n, i, timestep);
					}
				}
			}
			else
			{
				throw std::runtime_error("add neurons to the network before running it");
			}
			std::cout << "Done." << std::endl;
		}
		
		// update neuron status
		void update(Neuron* neuron, double time, float timestep)
		{
			if (generatedSpikes.empty() && !initialSpikes.empty())
			{
				spike s = initialSpikes.front();
				updateHelper(s, neuron, time, timestep,1);
			}
			else if (initialSpikes.empty() && !generatedSpikes.empty())
			{
				spike s = generatedSpikes.front();
				updateHelper(s, neuron, time, timestep,0);
			}
			else if (!initialSpikes.empty() && !generatedSpikes.empty())
			{
				if (initialSpikes.front().timestamp < generatedSpikes.front().timestamp)
				{
					spike s = initialSpikes.front();
					updateHelper(s, neuron, time, timestep,1);
				}
				else
				{
					spike s = generatedSpikes.front();
					updateHelper(s, neuron, time, timestep,0);
				}
			}
			else
			{
				neuron->update(time, timestep, spike({time, nullptr}), this);
			}
		}
		
		// helper for the update method
		void updateHelper(spike s, Neuron* neuron, double time, float timestep, int listSelector)
		{
			if (s.axon->postNeuron->getNeuronID() == neuron->getNeuronID())
			{
				
				if (s.timestamp <= time + (timestep/2))
				{
					neuron->update(time, timestep, s, this);

					if (listSelector == 0)
					{
						if (!generatedSpikes.empty())
						{
							generatedSpikes.pop_front();
						}
					}
					
					else if (listSelector == 1)
					{
						initialSpikes.pop_front();
					}
				}
				else
				{
					neuron->update(time, timestep, spike({time, nullptr}),this);
				}
			}
			else
			{
				neuron->update(time, timestep, spike({time, nullptr}),this);
			}
		}
		
		void globalLearningRuleMonitor()
		{
			for (auto& n: neurons)
			{
				for (auto& rule: n.getLearningRuleHandler())
				{
					if(StandardNetworkAddOn* globalRule = dynamic_cast<StandardNetworkAddOn*>(rule))
					{
						if (std::find(stdAddOns.begin(), stdAddOns.end(), dynamic_cast<StandardNetworkAddOn*>(rule)) == stdAddOns.end())
						{
							stdAddOns.emplace_back(dynamic_cast<StandardNetworkAddOn*>(rule));
						}
					}
				}
			}
		}
		
		// ----- IMPLEMENTATION VARIABLES -----
		std::deque<spike>                      initialSpikes;
        std::deque<spike>                      generatedSpikes;
        std::vector<StandardNetworkAddOn*>     stdAddOns;
        MainThreadNetworkAddOn*                thAddOn;
        std::vector<layer>                     layers;
		std::vector<Neuron>                    neurons;
		std::deque<label>                      trainingLabels;
		bool                                   learningStatus;
		double                                 learningOffSignal;
        int                                    maxDelay;
        std::string                            currentLabel;
    };
}
