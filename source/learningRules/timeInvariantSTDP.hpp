/*
 * timeInvariantSTDP.hpp
 * Adonis - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 08/02/2019
 *
 * Information: The timeInvariantSTDP learning rule works locally on a layer and does not depend on precise timing (the sign of the postsynaptic neuron time - postsynaptic neuron time only matters)
 * Adapted From: Thiele, J. C., Bichler, O., & Dupret, A. (2018). Event-Based, Timescale Invariant Unsupervised Online Deep Learning With STDP. Frontiers in Computational Neuroscience, 12. doi:10.3389/fncom.2018.00046
 */

#pragma once

#include "../globalLearningRuleHandler.hpp"
#include "../neurons/inputNeuron.hpp"
#include "../neurons/LIF.hpp"

namespace adonis {
	class Neuron;
	
	class TimeInvariantSTDP : public GlobalLearningRuleHandler {
        
	public:
		// ----- CONSTRUCTOR -----
        TimeInvariantSTDP(float _alpha_plus=1, float _alpha_minus=-8, float _beta_plus=3, float _beta_minus=0) :
                alpha_plus(_alpha_plus),
                alpha_minus(_alpha_minus),
                beta_plus(_beta_plus),
                beta_minus(_beta_minus) {}
		
		// ----- PUBLIC METHODS -----
        virtual void onStart(Network* network) override{
            // error handling
            for (auto& n: network->getNeurons()) {
                for (auto& rule: n->getLearningRuleHandler()) {
                    if(rule == this) {
                        if (n->getLayerID() == 0) {
                            throw std::logic_error("the STDP learning rule has to be on a postsynaptic layer");
                        }
                    }
                }
            }
        }
        
		virtual void learn(double timestamp, axon* a, Network* network) override {
            for (auto& preAxon: a->postNeuron->getPreAxons()) {
                // Long term potentiation for all presynaptic neurons that spiked
                if (timestamp >= preAxon->preNeuron->getPreviousSpikeTime() && preAxon->preNeuron->getPreviousSpikeTime() > preAxon->postNeuron->getPreviousSpikeTime()) {
                    float delta_weight = alpha_plus * std::exp(- beta_plus * preAxon->weight * preAxon->postNeuron->getMembraneResistance());
                    preAxon->weight += delta_weight*(1./preAxon->postNeuron->getMembraneResistance());
                    std::cout << "LTP " << delta_weight << " " << preAxon->weight << std::endl;
                // Long term depression for all presynaptic neurons neurons that didn't spike
                } else {
                    float delta_weight = alpha_minus * std::exp(- beta_minus * (1 - preAxon->weight * preAxon->postNeuron->getMembraneResistance()));
                    if (preAxon->weight > 0) {
                        preAxon->weight -= delta_weight*(1./preAxon->postNeuron->getMembraneResistance());
                        if (preAxon->weight < 0) {
                            preAxon->weight = 0;
                        }
                    }
                    std::cout << "LTD " << delta_weight << " " << preAxon->weight << std::endl;
                }
            }
		}
		
	protected:
	
		// ----- LEARNING RULE PARAMETERS -----
        float alpha_plus;
        float alpha_minus;
        float beta_plus;
        float beta_minus;
	};
}
