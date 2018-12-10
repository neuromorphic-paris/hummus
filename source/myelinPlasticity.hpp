/*
 * myelinPlasticity.hpp
 * Adonis_c - clock-driven spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 25/09/2018
 *
 * Information: The MyelinPlasticity class
 */

#pragma once

namespace adonis_c
{
	class Neuron;
	
	class MyelinPlasticity : public LearningRuleHandler
	{
	public:
		// ----- CONSTRUCTOR -----
		MyelinPlasticity(float _alpha=1, float _lambda=1) :
		alpha(_alpha),
		lambda(_lambda)
		{}
		
		// ----- PUBLIC METHODS -----
		virtual void learn(double timestamp, Neuron* neuron, Network* network) override
		{
			std::vector<double> timeDifferences;
			std::vector<int16_t> plasticID;
			std::vector<std::vector<int16_t>> plasticCoordinates(4);
			#ifndef NDEBUG
			std::cout << "New learning epoch at t=" << timestamp << std::endl;
			#endif
			
			for (auto inputAxon: neuron->getPreAxons())
			{
				// selecting plastic neurons
				if (inputAxon->preNeuron->getEligibilityTrace() > 0.1)
				{
					plasticID.push_back(inputAxon->preNeuron->getNeuronID());
					plasticCoordinates[0].push_back(inputAxon->preNeuron->getX());
					plasticCoordinates[1].push_back(inputAxon->preNeuron->getY());
					plasticCoordinates[2].push_back(inputAxon->preNeuron->getRfRow());
					plasticCoordinates[3].push_back(inputAxon->preNeuron->getRfCol());
					
					float change = 0;
					
					timeDifferences.push_back(timestamp - inputAxon->lastInputTime - inputAxon->delay);
					if (timeDifferences.back() > 0)
					{
						change = lambda*(neuron->getInputResistance()/(neuron->getDecayCurrent()-neuron->getDecayPotential())) * neuron->getCurrent() * (std::exp(-alpha*timeDifferences.back()/neuron->getDecayCurrent()) - std::exp(-alpha*timeDifferences.back()/neuron->getDecayPotential()))*neuron->getSynapticEfficacy();
						inputAxon->delay += change;
						#ifndef NDEBUG
						std::cout << inputAxon->preNeuron->getLayerID() << " " << inputAxon->preNeuron->getNeuronID() << " " << inputAxon->postNeuron->getNeuronID() << " time difference: " << timeDifferences.back() << " delay change: " << change << std::endl;
						#endif
					}

					else if (timeDifferences.back() < 0)
					{
						change = -lambda*((neuron->getInputResistance()/(neuron->getDecayCurrent()-neuron->getDecayPotential())) * neuron->getCurrent() * (std::exp(alpha*timeDifferences.back()/neuron->getDecayCurrent()) - std::exp(alpha*timeDifferences.back()/neuron->getDecayPotential())))*neuron->getSynapticEfficacy();
						inputAxon->delay += change;
						#ifndef NDEBUG
						std::cout << inputAxon->preNeuron->getLayerID() << " " << inputAxon->preNeuron->getNeuronID() << " " << inputAxon->postNeuron->getNeuronID() << " time difference: " << timeDifferences.back() << " delay change: " << change << std::endl;
						#endif
					}
					neuron->setSynapticEfficacy(-std::exp(-std::pow(timeDifferences.back(),2))+1);

				}
			}
			
			for (auto addon: network->getStandardAddOns())
			{
				addon->learningEpoch(timestamp, network, neuron, timeDifferences, plasticCoordinates);
			}
			
			if (network->getMainThreadAddOn())
			{
				network->getMainThreadAddOn()->learningEpoch(timestamp, network, neuron, timeDifferences, plasticCoordinates);
			}
		}
		
	protected:
	
		// ----- LEARNING RULE PARAMETERS -----
		float alpha;
		float lambda;
	};
}
