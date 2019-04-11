/*
 * LIF.hpp
 * Hummus - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 21/01/2019
 *
 * Information: leaky integrate and fire (LIF) neuron model with current dynamics.
 *
 * NEURON TYPE 1 (in JSON SAVE FILE)
 */

#pragma once

#include "../core.hpp"
#include "../dependencies/json.hpp"
#include "../synapticKernels/exponential.hpp"
#include "../synapticKernels/dirac.hpp"
#include "../synapticKernels/step.hpp"

namespace hummus {
    
	class LIF : public Neuron {
        
	public:
		// ----- CONSTRUCTOR AND DESTRUCTOR -----
		LIF(int _neuronID, int _layerID, int _sublayerID, std::pair<int, int> _rfCoordinates,  std::pair<int, int> _xyCoordinates, std::vector<LearningRuleHandler*> _learningRules, SynapticKernelHandler* _synapticKernel, bool _homeostasis=false, float _decayPotential=20, int _refractoryPeriod=3, bool _wta=false, bool _burstingActivity=false, float _eligibilityDecay=20, float _decayWeight=0, float _decayHomeostasis=20, float _homeostasisBeta=0.1, float _threshold=-50, float _restingPotential=-70, float _externalCurrent=100) :
                Neuron(_neuronID, _layerID, _sublayerID, _rfCoordinates, _xyCoordinates, _learningRules, _synapticKernel, _eligibilityDecay, _threshold, _restingPotential),
                refractoryPeriod(_refractoryPeriod),
                decayPotential(_decayPotential),
                externalCurrent(_externalCurrent),
                current(0),
                active(true),
                burstingActivity(_burstingActivity),
                homeostasis(_homeostasis),
                restingThreshold(_threshold),
                decayWeight(_decayWeight),
                decayHomeostasis(_decayHomeostasis),
                homeostasisBeta(_homeostasisBeta),
                inhibited(false),
                inhibitionTime(0),
                wta(_wta) {
					
			// error handling
    	    if (decayPotential <= 0) {
                throw std::logic_error("The potential decay cannot less than or equal to 0");
            }
					
            if (decayPotential == synapticKernel->getSynapseTimeConstant()) {
                throw std::logic_error("The potential decay and the synapse time constant cannot be equal");
            }
					
            // LIF neuron type == 1 (for JSON save)
            neuronType = 1;
		}
		
		virtual ~LIF(){}
		
		// ----- PUBLIC LIF METHODS -----
		virtual void initialisation(Network* network) override {
            // checking which synaptic kernel was chosen in the asynchronous network 
            if (network->getNetworkType() == true) {
            	if (Exponential *kernel = dynamic_cast<Exponential*>(synapticKernel)) {
                	throw std::logic_error("the event-based LIF neuron does not work with the Exponential kernel, as the biexponential model it is based on, does not have an analytical solution");
				}
            }
            
            // checking if any children of the globalLearningRuleHandler class were initialised and adding them to the Addons vector
            for (auto& rule: learningRules) {
                if (AddOn *globalRule = dynamic_cast<AddOn*>(rule)) {
                    if (std::find(network->getAddOns().begin(), network->getAddOns().end(), dynamic_cast<AddOn*>(rule)) == network->getAddOns().end()) {
                        network->getAddOns().emplace_back(dynamic_cast<AddOn*>(rule));
                    }
                }
            }
		}
        
		virtual void update(double timestamp, synapse* a, Network* network, spikeType type) override {
            if (type == spikeType::normal) {
                // checking if the neuron is inhibited
                if (inhibited && timestamp - inhibitionTime >= refractoryPeriod) {
                    inhibited = false;
                }
                
                // checking if the neuron is in a refractory period
                if (timestamp - previousSpikeTime >= refractoryPeriod) {
                    active = true;
                }
				
				// updating the current
                current = synapticKernel->updateCurrent(timestamp, 0, previousInputTime, current);
				
                // eligibility trace decay
                eligibilityTrace *= std::exp(-(timestamp-previousInputTime)*adaptation/eligibilityDecay);
                
                // potential decay
                potential = restingPotential + (potential-restingPotential)*std::exp(-(timestamp-previousInputTime)*adaptation/decayPotential);
                
                // threshold decay
                if (homeostasis) {
                    threshold = restingThreshold + (threshold-restingThreshold)*std::exp(-(timestamp-previousInputTime)*adaptation/decayHomeostasis);
                }
                
                // synapse weight decay - synaptic pruning
                if (decayWeight != 0) {
                    a->weight *= std::exp(-(timestamp-previousInputTime)*synapticEfficacy/decayWeight);
                }
            
                if (active && !inhibited) {
					// calculating the potential
                    potential = restingPotential + current * (1 - std::exp(-(timestamp-previousInputTime)/decayPotential)) + (potential - restingPotential) * std::exp(-(timestamp-previousInputTime)/decayPotential);
                    
                    // updating the threshold
                    if (homeostasis) {
                        threshold += homeostasisBeta/decayHomeostasis;
                    }
					
                    // synaptic integration
					current = synapticKernel->integrateSpike(current, externalCurrent, a->weight);

                    if (network->getVerbose() == 2) {
                        std::cout << "t=" << timestamp << " " << (a->preNeuron ? a->preNeuron->getNeuronID() : -1) << "->" << neuronID << " w=" << a->weight << " d=" << a->delay <<" V=" << potential << " Vth=" << threshold << " layer=" << layerID << " --> EMITTED" << std::endl;
                    }
                    
                    for (auto addon: network->getAddOns()) {
                        if (potential < threshold) {
                            addon->incomingSpike(timestamp, a, network);
                        }
                    }
                    if (network->getMainThreadAddOn()) {
                        network->getMainThreadAddOn()->incomingSpike(timestamp, a, network);
                    }
					
                    if (a->weight >= 0) {
                        // calculating time at which potential = threshold
                        double predictedTimestamp = decayPotential * (- std::log( - threshold + restingPotential + current) + std::log( current - potential + restingPotential)) + timestamp;
                        
                        if (predictedTimestamp > timestamp && predictedTimestamp <= timestamp + synapticKernel->getSynapseTimeConstant()) {
                            network->injectPredictedSpike(spike{predictedTimestamp, a, spikeType::prediction}, spikeType::prediction);
                        } else {
                            network->injectPredictedSpike(spike{timestamp + synapticKernel->getSynapseTimeConstant(), a, spikeType::endOfIntegration}, spikeType::endOfIntegration);
                        }
                    } else {
                        potential = restingPotential + current * (1 - std::exp(-(timestamp-previousInputTime)/decayPotential)) + (potential - restingPotential);
                    }
                }
            } else if (type == spikeType::prediction) {
                if (active && !inhibited) {
                    potential = restingPotential + current * (1 - std::exp(-(timestamp-previousInputTime)/decayPotential)) + (potential - restingPotential);
                }
            } else if (type == spikeType::endOfIntegration) {
                if (active && !inhibited) {
                    potential = restingPotential + current * (1 - std::exp(-synapticKernel->getSynapseTimeConstant()/decayPotential)) + (potential - restingPotential) * std::exp(-synapticKernel->getSynapseTimeConstant()/decayPotential);
                }
            }
        
            if (network->getMainThreadAddOn()) {
                network->getMainThreadAddOn()->statusUpdate(timestamp, a, network);
            }

            if (potential >= threshold) {
                eligibilityTrace = 1;

                if (network->getVerbose() == 2) {
                    std::cout << "t=" << timestamp << " " << (a->preNeuron ? a->preNeuron->getNeuronID() : -1) << "->" << neuronID << " w=" << a->weight << " d=" << a->delay <<" V=" << potential << " Vth=" << threshold << " layer=" << layerID << " --> SPIKED" << std::endl;
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
                if (!burstingActivity) {
                    current = 0;
                }
                active = false;
                
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->statusUpdate(timestamp, a, network);
                }
            }
            
            // updating the timestamp when a synapse was propagating a spike
            previousInputTime = timestamp;
            a->previousInputTime = timestamp;
		}
		
		virtual void updateSync(double timestamp, synapse* a, Network* network, double timestep) override {
            // handling multiple spikes at the same timestamp (to prevent excessive decay)
            if (timestamp != 0 && timestamp - previousSpikeTime == 0) {
                timestep = 0;
            }
            
            // checking if the neuron is inhibited
			if (inhibited && timestamp - inhibitionTime >= refractoryPeriod) {
				inhibited = false;
			}
            
            // checking if the neuron is in a refractory period
            if (timestamp - previousSpikeTime >= refractoryPeriod) {
                active = true;
            }
			
			// updating the current
            current = synapticKernel->updateCurrent(timestamp, timestep, previousInputTime, current);
            
            // eligibility trace decay
            eligibilityTrace *= std::exp(-timestep*adaptation/eligibilityDecay);
            
			// potential decay
            potential = restingPotential + (potential-restingPotential)*std::exp(-timestep*adaptation/decayPotential);
            
			// threshold decay
			if (homeostasis) {
                threshold = restingThreshold + (threshold-restingThreshold)*std::exp(-timestep*adaptation/decayHomeostasis);
			}

            if (a) {
                // synapse weight decay - synaptic pruning
                if (decayWeight != 0) {
                    a->weight *= std::exp(-(timestamp-previousInputTime)*synapticEfficacy/decayWeight);
                }
            }
                
			// neuron inactive during refractory period
			if (active && !inhibited) {
				if (a) {
					// updating the threshold
					if (homeostasis) {
						threshold += homeostasisBeta/decayHomeostasis;
					}
                    
                    // integrating spike
                    current = synapticKernel->integrateSpike(current, externalCurrent, a->weight);

					activeSynapse = a;
                    
                    // updating the timestamp when a synapse was propagating a spike
                    previousInputTime = timestamp;
					a->previousInputTime = timestamp;
                    
                    if (network->getVerbose() == 2) {
                        std::cout << "t=" << timestamp << " " << (a->preNeuron ? a->preNeuron->getNeuronID() : -1) << "->" << neuronID << " w=" << a->weight << " d=" << a->delay <<" V=" << potential << " Vth=" << threshold << " layer=" << layerID << " --> EMITTED" << std::endl;
                    }
                    
                    for (auto addon: network->getAddOns()) {
                        if (potential < threshold) {
                            addon->incomingSpike(timestamp, a, network);
                        }
                    }
                    if (network->getMainThreadAddOn()) {
                        network->getMainThreadAddOn()->incomingSpike(timestamp, a, network);
                    }
				}
				
                potential += current * (1 - std::exp(-timestep/decayPotential));
            }
            
            if (a) {
                if (network->getMainThreadAddOn()) {
                    network->getMainThreadAddOn()->statusUpdate(timestamp, a, network);
                }
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

			if (potential >= threshold) {
				eligibilityTrace = 1;
                
                if (network->getVerbose() == 2) {
                    std::cout << "t=" << timestamp << " " << (activeSynapse->preNeuron ? activeSynapse->preNeuron->getNeuronID() : -1) << "->" << neuronID << " w=" << activeSynapse->weight << " d=" << activeSynapse->delay <<" V=" << potential << " Vth=" << threshold << " layer=" << layerID << " --> SPIKED" << std::endl;
                }

				for (auto addon: network->getAddOns()) {
					addon->neuronFired(timestamp, activeSynapse, network);
				}
				if (network->getMainThreadAddOn()) {
					network->getMainThreadAddOn()->neuronFired(timestamp, activeSynapse, network);
				}

				for (auto& p : postSynapses) {
                    network->injectGeneratedSpike(spike{timestamp + p->delay, p.get(), spikeType::normal});
				}

				requestLearning(timestamp, activeSynapse, network);

				previousSpikeTime = timestamp;
				potential = restingPotential;
				if (!burstingActivity) {
					current = 0;
				}
				active = false;
			}
		}
		
        virtual void resetNeuron(Network* network) override {
            // resetting parameters
            previousInputTime = 0;
            previousSpikeTime = 0;
            current = 0;
            potential = restingPotential;
            eligibilityTrace = 0;
            inhibited = false;
            active = true;
            threshold = restingThreshold;
        }
        
        // write neuron parameters in a JSON format
        virtual void toJson(nlohmann::json& output) override{
            // general neuron parameters
            output.push_back({
                {"Type",neuronType},
                {"layerID",layerID},
                {"sublayerID", sublayerID},
                {"receptiveFieldCoordinates", rfCoordinates},
                {"XYCoordinates", xyCoordinates},
                {"eligibilityDecay", eligibilityDecay},
                {"threshold", threshold},
                {"restingPotential", restingPotential},
                {"refractoryPeriod", refractoryPeriod},
                {"decayPotential", decayPotential},
                {"externalCurrent", externalCurrent},
                {"burstingActivity", burstingActivity},
                {"homeostasis", homeostasis},
                {"restingThreshold", restingThreshold},
                {"decayWeight", decayWeight},
                {"decayHomeostasis", decayHomeostasis},
                {"homeostasisBeta", homeostasisBeta},
                {"wta", wta},
                {"dendriticSynapses", nlohmann::json::array()},
                {"axonalSynapses", nlohmann::json::array()},
            });
            
            // dendritic synapses (preSynapse)
            auto& dendriticSynapses = output.back()["dendriticSynapses"];
            for (auto& preS: preSynapses) {
                dendriticSynapses.push_back({
                    {"weight", preS->weight},
                    {"delay", preS->delay},
                });
            }
            
            // axonal synapses (postSynapse)
            auto& axonalSynapses = output.back()["axonalSynapses"];
            for (auto& postS: postSynapses) {
                axonalSynapses.push_back({
                    {"postNeuronID", postS->postNeuron->getNeuronID()},
                    {"weight", postS->weight},
                    {"delay", postS->delay},
                });
            }
        }
        
		// ----- SETTERS AND GETTERS -----
		bool getActivity() const {
			return active;
		}
		
		float getDecayPotential() const {
            return decayPotential;
        }
		
        void setDecayPotential(float newDecayPotential) {
            decayPotential = newDecayPotential;
        }
		
        float getCurrent() const {
        	return current;
		}
		
		void setCurrent(float newCurrent) {
			current = newCurrent;
		}
		
		float getExternalCurrent() const {
			return externalCurrent;
		}
		
		void setExternalCurrent(float newCurrent) {
			externalCurrent = newCurrent;
		}
		
		void setInhibition(double timestamp, bool inhibitionStatus) {
			inhibitionTime = timestamp;
			inhibited = inhibitionStatus;
		}
		
        void setRefractoryPeriod(float newRefractoryPeriod) {
            refractoryPeriod = newRefractoryPeriod;
        }
        
        void setBurstingActivity(bool newBool) {
            burstingActivity = newBool;
        }
        
        void setHomeostasis(bool newBool) {
            homeostasis = newBool;
        }
        
        void setRestingThreshold(float newThres) {
            restingThreshold = newThres;
        }
        
        void setDecayWeight(float newDW) {
            decayWeight = newDW;
        }
        
        void setDecayHomeostasis(float newDH) {
            decayHomeostasis = newDH;
        }
        
        void setHomeostasisBeta(float newHB) {
            homeostasisBeta = newHB;
        }
        
        void setWTA(bool newBool) {
            wta = newBool;
        }
        
	protected:
        
        // winner-take-all algorithm
		virtual void WTA(double timestamp, Network* network) override {
            for (auto& sub: network->getLayers()[layerID].sublayers) {
                // intra-sublayer hard WTA
                if (sub.ID == sublayerID) {
                    for (auto& n: sub.neurons) {
                        if (network->getNeurons()[n]->getNeuronID() != neuronID && network->getNeurons()[n]->getRfCoordinates() == rfCoordinates) {
                            network->getNeurons()[n]->setPotential(restingPotential);
                            if (LIF* neuron = dynamic_cast<LIF*>(network->getNeurons()[n].get())) {
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->current = 0;
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->inhibited = true;
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->inhibitionTime = timestamp;
                            }
                        }
                    }
                // inter-sublayer hard WTA
                } else {
                    for (auto& n: sub.neurons) {
                        if (network->getNeurons()[n]->getRfCoordinates() == rfCoordinates) {
                            network->getNeurons()[n]->setPotential(restingPotential);
                            if (LIF* neuron = dynamic_cast<LIF*>(network->getNeurons()[n].get())) {
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->current = 0;
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->inhibited = true;
                                dynamic_cast<LIF*>(network->getNeurons()[n].get())->inhibitionTime = timestamp;
                            }
                        }
                    }
                }
            }
		}
		
        // loops through any learning rules and activates them
        virtual void requestLearning(double timestamp, synapse* a, Network* network) override {
            if (network->getLearningStatus()) {
                if (!learningRules.empty()) {
                    for (auto& rule: learningRules) {
                        rule->learn(timestamp, a, network);
                    }
                }
            }
            if (wta) {
                WTA(timestamp, network);
            }
        }
        
		// ----- LIF PARAMETERS -----
        float                                    decayWeight;
		float                                    decayPotential;
        float                                    current;
		bool                                     active;
		bool                                     inhibited;
		double                                   inhibitionTime;
		float                                    refractoryPeriod;
		float                                    externalCurrent;
		bool                                     burstingActivity;
		bool                                     homeostasis;
		float                                    restingThreshold;
		float                                    decayHomeostasis;
		float                                    homeostasisBeta;
		bool                                     wta;
		synapse*                                 activeSynapse;
	};
}
