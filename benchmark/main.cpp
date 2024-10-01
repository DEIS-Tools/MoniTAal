/*
 * Copyright Thomas M. Grosen 
 * Created on 19/05/2022
 */

/*
 * This file is part of MoniTAal
 *
 * MoniTAal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MoniTAal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MoniTAal. If not, see <https://www.gnu.org/licenses/>.
 */

#include "monitaal/Parser.h"
#include "monitaal/Fixpoint.h"
#include "monitaal/state.h"
#include "monitaal/Monitor.h"
#include "monitaal/EventParser.h"

#include "gear_controller_model.h"

#include <cstdlib>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace po = boost::program_options;
using namespace monitaal;

struct latency_t {
    symb_time_t lower = 0,
                upper = 0;
    
    friend std::ostream& operator<<(std::ostream& os, latency_t const& l) {
        return os << "[" << l.lower << ", " << l.upper << "]";
    }
    friend std::istream& operator>>(std::istream& is, latency_t& l) {
        return is >> std::skipws >> l.lower >> std::skipws >> l.upper;
    }
};

void run_gearcontroller(int observation_lenght, interval_t latency, symb_time_t jitter, bool inclusion) {
    settings_t setting{inclusion, inclusion, {latency.first, latency.second}, jitter};
    std::vector<Delay_monitor> monitors = {
        Delay_monitor(Parser::parse_data(gear_controller_properties, "CloseClutch"), Parser::parse_data(gear_controller_properties, "NotCloseClutch"), setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "OpenClutch"), Parser::parse_data(gear_controller_properties, "NotOpenClutch"), setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "ReqSet"), Parser::parse_data(gear_controller_properties, "NotReqSet"), setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "ReqNeu"), Parser::parse_data(gear_controller_properties, "NotReqNeu"), setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "SpeedSet"), Parser::parse_data(gear_controller_properties, "NotSpeedSet"), setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "test1"), Parser::parse_data(gear_controller_properties, "Nottest1"), setting)
    };

    auto size = monitors.size();
    bool is_firm = false;
    int event_counter = 0;

    int tmp = 0;
    int max_states = 0;
    int max_response_time = 0;
    int response_time = 0;
    int time_horizon = 0;

    std::stringstream input_stream(gear_controller_input, std::ios::in);

    std::vector<timed_input_t> events;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);

    while ((not is_firm) && (observation_lenght == 0 || event_counter < observation_lenght)) {

        events = EventParser::parse_input(&input_stream, 1);
        if (events.size() == 0) break;
        
        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < size; ++i) {
            monitors[i].input(events);
        }
        t2 = std::chrono::high_resolution_clock::now();
        
        tmp = time.count();
        time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        response_time = time.count() - tmp;
        max_response_time = response_time > max_response_time ? response_time : max_response_time;
        // std::cout << "Response time: " << response_time << "ns\n";

        tmp = max_states;
        max_states = 0;

        for (int i = 0; i < size; ++i) {
            max_states += monitors[i].positive_state_estimate().size() + 
                          monitors[i].negative_state_estimate().size();
        }

        max_states = tmp > max_states ? tmp : max_states;
        
        event_counter += events.size();
        time_horizon = events[events.size()-1].time.second;
    }

    std::cout << "Monitored " << event_counter << " events in " << time.count() << 
                 "ns\nMax states: "<< max_states << "\nmax response: "<< max_response_time << "ns\nTime Horizon: " << time_horizon <<"\nMemory: " << sizeof(monitors) <<"\nMonitor verdicts are\n";
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-benchmark --pos <name> <path> --neg <name> <path> --input <path>")
            ("length", po::value<int>()->default_value(0, "0"), "Bound on the number of observations. 0 means no bound")
            ("latency", po::value<std::vector<symb_time_t>>()->multitoken()->default_value({0, 0}, "0 0"), "Specify latency upper and lower bound parameters")
            ("jitter", po::value<symb_time_t>()->default_value(0, "0"), "Specify the jitter upper bound parameter")
            ("inclusion", "Enable inclusion and inactive clock abstraction")
            ("gear-controller", "Run Gear controller benchmark");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

    if (vm.count("help")) {
        std::cout << options << std::endl;
        return 1;
    }

    try {
        po::notify(vm);
    } catch (boost::wrapexcept<po::required_option>& e) {
        std::cerr << "Error: Some required fields not given\n";
        exit(-1);
    }

    auto latency = vm["latency"].as<std::vector<symb_time_t>>(); 
    
    if (latency.size() != 2) {
        std::cerr << "Error: Latency needs exactly two arguments (lower and upper bound)\n";
        exit(-1);
    }
    
    if (vm.count("gear-controller"))
        run_gearcontroller(vm["length"].as<int>(), {latency[0], latency[1]}, vm["jitter"].as<symb_time_t>(), vm.count("inclusion"));


    return 0;
}
