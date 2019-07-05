/*
 * mp_rates.cpp
 * Hummus - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 14/01/2019
 *
 * Information: figuring out how to work with rates in the context of the myelin plasticity rule
 */

#include <iostream>

#include "../source/core.hpp"
#include "../source/dataParser.hpp"

#include "../source/randomDistributions/normal.hpp"

#include "../source/GUI/qt/qtDisplay.hpp"

#include "../source/neurons/parrot.hpp"
#include "../source/neurons/decisionMaking.hpp"
#include "../source/neurons/LIF.hpp"

#include "../source/addons/spikeLogger.hpp"
#include "../source/addons/potentialLogger.hpp"
#include "../source/addons/classificationLogger.hpp"
#include "../source/addons/weightMaps.hpp"
#include "../source/addons/analysis.hpp"

#include "../source/learningRules/myelinPlasticity.hpp"

#include "../source/synapses/dirac.hpp"
#include "../source/synapses/pulse.hpp"
#include "../source/synapses/exponential.hpp"

int main(int argc, char** argv) {
    hummus::Network network;
    network.makeAddon<hummus::MyelinPlasticityLogger>("rates_mpLog.bin");
    
    auto& display = network.makeGUI<hummus::QtDisplay>();
    auto& mp = network.makeAddon<hummus::MyelinPlasticity>();
    
    auto input = network.makeLayer<hummus::Parrot>(3, {});
    auto output = network.makeLayer<hummus::LIF>(1, {&mp}, true, 200, 10, 1, false);

    network.allToAll<hummus::Exponential>(input, output, 1, hummus::Normal(1./3, 0, 5, 3), 100);
    
    int repetitions = 10;
    int time_between_spikes = 100;
    int runtime = repetitions*time_between_spikes+10;
    
    for (auto i=0; i<repetitions; i++) {
        network.injectSpike(0, 10+time_between_spikes*i);
        network.injectSpike(0, 11.5+time_between_spikes*i);
        network.injectSpike(1, 15+time_between_spikes*i);
        network.injectSpike(2, 20+time_between_spikes*i);
    }

    display.setTimeWindow(200);
    display.trackNeuron(3);

    network.verbosity(1);
    network.run(runtime, 0.1);
    
    return 0;
}
