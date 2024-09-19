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

#include <cstdlib>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace po = boost::program_options;
using namespace monitaal;

void run_gearcontroller2(int limit) {
    char* prop = "gear-control-properties.xml";
    settings_t setting{false, false, {0, 100}, 5};
    std::vector<Delay_monitor> monitors = {
        Delay_monitor(Parser::parse(prop, "CloseClutch"), Parser::parse(prop, "NotCloseClutch"), setting),
        Delay_monitor(Parser::parse(prop, "OpenClutch"), Parser::parse(prop, "NotOpenClutch"), setting),
        Delay_monitor(Parser::parse(prop, "ReqSet"), Parser::parse(prop, "NotReqSet"), setting),
        Delay_monitor(Parser::parse(prop, "ReqNeu"), Parser::parse(prop, "NotReqNeu"), setting),
        Delay_monitor(Parser::parse(prop, "SpeedSet"), Parser::parse(prop, "NotSpeedSet"), setting),
        Delay_monitor(Parser::parse(prop, "test1"), Parser::parse(prop, "Nottest1"), setting)
    };

    auto size = monitors.size();
    bool is_firm = false;
    int event_counter = 0;

    int tmp = 0;
    int max_states = 0;
    int max_response_time = 0;
    int response_time = 0;
    int time_horizon = 0;

    std::filebuf fb;
    fb.open("gear-control-input3.txt", std::ios::in);
    std::istream stream(&fb);
    std::vector<timed_input_t> events;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

    while ((not is_firm) && (limit == 0 || event_counter < limit)) {

        events = EventParser::parse_input(&stream, 1);
        if (events.size() == 0) break;
        
        for (int i = 0; i < size; ++i) {
            monitors[i].input(events);
        }
        event_counter += events.size();
        
    }
    t2 = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

    fb.close();
    std::cout << "Monitored " << event_counter << " events in " << time.count() << "ms\n";
}

void run_gearcontroller(int limit) {
    char* prop = "gear-control-properties.xml";
    settings_t setting{false, false, {0, 0}, 0};
    std::vector<Delay_monitor> monitors = {
        Delay_monitor(Parser::parse(prop, "CloseClutch"), Parser::parse(prop, "NotCloseClutch"), setting),
        Delay_monitor(Parser::parse(prop, "OpenClutch"), Parser::parse(prop, "NotOpenClutch"), setting),
        Delay_monitor(Parser::parse(prop, "ReqSet"), Parser::parse(prop, "NotReqSet"), setting),
        Delay_monitor(Parser::parse(prop, "ReqNeu"), Parser::parse(prop, "NotReqNeu"), setting),
        Delay_monitor(Parser::parse(prop, "SpeedSet"), Parser::parse(prop, "NotSpeedSet"), setting),
        Delay_monitor(Parser::parse(prop, "test1"), Parser::parse(prop, "Nottest1"), setting)
    };

    auto size = monitors.size();
    bool is_firm = false;
    int event_counter = 0;

    int tmp = 0;
    int max_states = 0;
    int max_response_time = 0;
    int response_time = 0;
    int time_horizon = 0;

    std::filebuf fb;
    fb.open("gear-control-input3.txt", std::ios::in);
    std::istream stream(&fb);
    std::vector<timed_input_t> events;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);

    while ((not is_firm) && (limit == 0 || event_counter < limit)) {

        events = EventParser::parse_input(&stream, 1);
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

    fb.close();
    std::cout << "Monitored " << event_counter << " events in " << time.count() << 
                 "ns\nMax states: "<< max_states << "\nmax response: "<< max_response_time << "ns\nTime Horizon: " << time_horizon <<"\nMemory: " << sizeof(monitors) <<"\nMonitor verdicts are\n";
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-benchmark --pos <name> <path> --neg <name> <path> --input <path>")
            ("G1",po::value<int>(), "Run Gear controller benchmark")
            ("G2",po::value<int>(), "Run Gear controller benchmark");

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

    if (vm.count("G1"))
        run_gearcontroller(vm["G1"].as<int>());
    if (vm.count("G2"))
        run_gearcontroller2(vm["G2"].as<int>());


    return 0;
}
