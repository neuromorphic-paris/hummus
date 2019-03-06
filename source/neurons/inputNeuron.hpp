/*
 * inputNeuron.hpp
 * Hummus - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 24/01/2019
 *
 * Information: input neurons take in spikes or events and instantly propagate them in the network. The potential does not decay.
 *
 * NEURON TYPE 0 (in JSON SAVE FILE)
 */

#pragma once

#include "../core.hpp"
#include "../dependencies/json.hpp"

namespace hummus {
	class InputNeuron : public Neuron {
        
	public:
		// ----- CONSTRUCTOR AND DESTRUCTOR -----
		InputNeuron(int16_t _neuronID, int16_t _layerID, int16_t _sublayerID, std::pair<int16_t, int16_t> _rfCoordinates,  std::pair<int16_t, int16_t> _xyCoordinates, std::vector<LearningRuleHandler*> _learningRuleHandler={}, int _refractoryPeriod=0, float _eligibilityDecay=20, float _threshold=-50, float _restingPotential=-70, float _membraneResistance=50e9) :
                Neuron(_neuronID, _layerID, _sublayerID, _rfCoordinates, _xyCoordinates, _learningRuleHandler, _eligibilityDecay, _threshold, _restingPotential, _membraneResistance),
                active(true),
                refractoryPeriod(_refractoryPeriod) {}
		
		virtual ~InputNeuron(){}
		
		// ----- PUBLIC INPUT NEURON METHODS -----
		void initialisation(Network* network) override {
			for (auto& rule: learningRuleHandler) {
				if(AddOn* globalRule = dynamic_cast<AddOn*>(rule)) {
					if (std::find(network->getAddOns().begin(), network->getAddOns().end(), dynamic_cast<AddOn*>(rule)) == network->getAddOns().end()) {
						network->getAddOns().emplace_back(dynamic_cast<AddOn*>(rule));
					}
				}
			}
		}
		
		void update(double timestamp, synapse* a, Network* network, spikeType type) override {
            
            // checking if the neuron is in a refractory period
            if (timestamp - previousSpikeTime >= refractoryPeriod) {
                active = true;
            }
            
            // eligibility trace decay
            eligibilityTrace *= std::exp(-(timestamp - previousSpikeTime)/eligibilityDecay);
            
            // instantly making the input neuron fire at every input spike
            if (active) {
                a->previousInputTime = timestamp;
                potential = threshold;
                eligibilityTrace = 1;
                
                #ifndef NDEBUG
                std::cout << "t=" << timestamp << " " << neuronID << " w=" << a->weight << " d=" << a->delay << " --> INPUT" << std::endl;
                #endif
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->incomingSpike(timestamp, a, network);
                }
                
                for (auto addon: network->getAddOns()) {
                    addon->neuronFired(timestamp, a, network);
                }
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->neuronFired(timestamp, a, network);
                }
                
                for (auto& p : postSynapses) {
                    network->injectGeneratedSpike(spike{timestamp + p->delay, p.get(), spikeType::normal});
                }
                
                requestLearning(timestamp, a, network);
                previousSpikeTime = timestamp;
                potential = restingPotential;
                active = false;
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->statusUpdate(timestamp, a, network);
                }
            }
		}
        
        void updateSync(double timestamp, synapse* a, Network* network, double timestep) override {
            
            if (timestamp != 0 && timestamp - previousSpikeTime == 0) {
                timestep = 0;
            }
            
            // checking if the neuron is in a refractory period
            if (timestamp - previousSpikeTime >= refractoryPeriod) {
                active = true;
            }
            
            // eligibility trace decay
            eligibilityTrace *= std::exp(-timestep/eligibilityDecay);
            
            if (a && active) {
                a->previousInputTime = timestamp;
                potential = threshold;
                eligibilityTrace = 1;
                
#ifndef NDEBUG
                std::cout << "t=" << timestamp << " " << neuronID << " w=" << a->weight << " d=" << a->delay << " --> INPUT" << std::endl;
#endif
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->incomingSpike(timestamp, a, network);
                }
                
                for (auto addon: network->getAddOns()) {
                    addon->neuronFired(timestamp, a, network);
                }
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->neuronFired(timestamp, a, network);
                }
                
                for (auto& p : postSynapses) {
                    network->injectGeneratedSpike(spike{timestamp + p->delay, p.get(), spikeType::normal});
                }
                
                requestLearning(timestamp, a, network);
                previousSpikeTime = timestamp;
                potential = restingPotential;
                active = false;
            } else {
                if (timestep > 0) {
                    for (auto addon: network->getAddOns()) {
                        addon->timestep(timestamp, network, this);
                    }
                    if (network->getMainThreadAddOn()) {
                        network->getMainThreadAddOn()->timestep(timestamp, network, this);
                    }
                }
            }
        }
        
        // write neuron parameters in a JSON format
        virtual void toJson(nlohmann::json& output) override{
            // general neuron parameters
            output.push_back({
                {"ID",neuronID},
                {"layerID",layerID},
                {"sublayerID", sublayerID},
                {"receptiveFieldCoordinates", rfCoordinates},
                {"XYCoordinates", xyCoordinates},
                {"eligibilityDecay", eligibilityDecay},
                {"threshold", threshold},
                {"restingPotential", restingPotential},
                {"resistance", membraneResistance},
                {"refractoryPeriod", refractoryPeriod},
                {"dendriticSynapses", nlohmann::json::array()},
                {"axonalSynapses", nlohmann::json::array()},
            });
            
            // dendritic synapses (preSynapse)
            auto& dendriticSynapses = output.back()["dendriticSynapses"];
            for (auto& preS: preSynapses) {
                dendriticSynapses.push_back({
                    {"preNeuronID",preS->preNeuron->getNeuronID()},
                    {"postNeuronID", preS->postNeuron->getNeuronID()},
                    {"weight", preS->weight},
                    {"delay", preS->delay},
                });
            }
            
            // axonal synapses (postSynapse)
            auto& axonalSynapses = output.back()["axonalSynapses"];
            for (auto& postS: postSynapses) {
                axonalSynapses.push_back({
                    {"preNeuronID",postS->preNeuron->getNeuronID()},
                    {"postNeuronID", postS->postNeuron->getNeuronID()},
                    {"weight", postS->weight},
                    {"delay", postS->delay},
                });
            }
        }
        
    protected:
        
        // loops through any learning rules and activates them
        void requestLearning(double timestamp, synapse* a ,Network* network) override {
            if (network->getLearningStatus()) {
                if (!learningRuleHandler.empty()) {
                    for (auto& learningRule: learningRuleHandler) {
                        learningRule->learn(timestamp, a, network);
                    }
                }
            }
        }
        
        // ----- INPUT NEURON PARAMETERS -----
        float refractoryPeriod;
        bool  active;
        
	};
}
