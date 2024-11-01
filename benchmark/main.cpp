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
#include "b_live_a_freq.h"

#include <cstdlib>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace po = boost::program_options;
using namespace monitaal;

struct benchmark_setting {
    int trace_bound = 1;
    bool inclusion = true;
    interval_t latency = {0, 0};
    symb_time_t jitter = 0;
    std::vector<std::string> div_alphabet = {};
};


void b_live_a_freq_concrete(benchmark_setting& setting) {
    TA pos = Parser::parse_data(b_live_a_freq_model, "positive");
    TA neg = Parser::parse_data(b_live_a_freq_model, "negative");

    if (setting.div_alphabet.size() > 0) {
        auto div = TA::time_divergence_ta(setting.div_alphabet, true);
        pos.intersection(div);
        neg.intersection(div);
    }

    settings_t monitor_setting(setting.inclusion, setting.inclusion, setting.latency, setting.jitter);

    Concrete_monitor monitor(pos, neg, monitor_setting);
    
    
    int max_states = 0, tmp = 0;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t2);

    for (int i = 0; i < setting.trace_bound-1; ++i) {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input(concrete_input(i, "a"));
        t2 = std::chrono::high_resolution_clock::now();

        res_tmp = ms_int.count();
        ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;
    }
    t1 = std::chrono::high_resolution_clock::now();
    monitor.input({11+setting.trace_bound, "b"});
    t2 = std::chrono::high_resolution_clock::now();
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << "\nMonitored " << (setting.trace_bound+1) << " events\n";
    return;
}

void b_live_a_freq_interval(benchmark_setting& setting) {
    TA pos = Parser::parse_data(b_live_a_freq_model, "positive");
    TA neg = Parser::parse_data(b_live_a_freq_model, "negative");

    if (setting.div_alphabet.size() > 0) {
        auto div = TA::time_divergence_ta(setting.div_alphabet, true);
        pos.intersection(div);
        neg.intersection(div);
    }

    settings_t monitor_setting(setting.inclusion, setting.inclusion, setting.latency, setting.jitter);

    Interval_monitor monitor(pos, neg, monitor_setting);
    bool overlap = true;
   
    int max_states = 0, tmp;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t2);

    for (int i = 0; i < setting.trace_bound-1; ++i) {
        if (overlap) {
            interval_input e({0+i, 10+i}, "a");
            t1 = std::chrono::high_resolution_clock::now();
            monitor.input(e);
            t2 = std::chrono::high_resolution_clock::now();
        } else {
            interval_input e({0, 10}, "a");
            t1 = std::chrono::high_resolution_clock::now();
            monitor.input(e);
            t2 = std::chrono::high_resolution_clock::now();
        }
        res_tmp = ms_int.count();
        ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;
    }
    if (overlap) {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input({{30+setting.trace_bound,30+setting.trace_bound}, "b"});
        t2 = std::chrono::high_resolution_clock::now();
    } else {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input({{30,30}, "b"});
        t2 = std::chrono::high_resolution_clock::now();
    }
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << "\nMonitored " << (setting.trace_bound+1) << " events\n";
    return;
}

void run_gearcontroller(benchmark_setting& setting) {
    settings_t monitor_setting{setting.inclusion, setting.inclusion, setting.latency, setting.jitter};

    std::vector<Delay_monitor> monitors = {
        Delay_monitor(Parser::parse_data(gear_controller_properties, "CloseClutch"), Parser::parse_data(gear_controller_properties, "NotCloseClutch"), monitor_setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "OpenClutch"), Parser::parse_data(gear_controller_properties, "NotOpenClutch"), monitor_setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "ReqSet"), Parser::parse_data(gear_controller_properties, "NotReqSet"), monitor_setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "ReqNeu"), Parser::parse_data(gear_controller_properties, "NotReqNeu"), monitor_setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "SpeedSet"), Parser::parse_data(gear_controller_properties, "NotSpeedSet"), monitor_setting),
        Delay_monitor(Parser::parse_data(gear_controller_properties, "test1"), Parser::parse_data(gear_controller_properties, "Nottest1"), monitor_setting)
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

    while ((not is_firm) && (setting.trace_bound == 0 || event_counter < setting.trace_bound)) {

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
            ("inclusion", "Enable inclusion and inactive clock abstraction")
            ("div,d", po::value<std::vector<std::string>>()->multitoken()->default_value({}, "Empty"), "<list of labels> : Take time divergence into account.")
            ("latency", po::value<std::vector<symb_time_t>>()->multitoken()->default_value({0, 0}, "0 0"), "Specify latency upper and lower bound parameters")
            ("jitter", po::value<symb_time_t>()->default_value(0, "0"), "Specify the jitter upper bound parameter")
            ("gear-controller", "Run Gear controller benchmark")
            ("b-live-a-freq-int","Run the b-liveness & a-frequency benchmark with interval time")
            ("b-live-a-freq-con","Run the b-liveness & a-frequency benchmark with concrete time");

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


    benchmark_setting setting;
    setting.trace_bound = vm["length"].as<int>();
    setting.jitter = vm["jitter"].as<symb_time_t>();
    setting.inclusion = vm.count("inclusion");
    setting.latency = {latency[0], latency[1]};
    setting.div_alphabet = vm["div"].as<std::vector<std::string>>();

    
    if (vm.count("gear-controller"))
        run_gearcontroller(setting);
    if (vm.count("b-live-a-freq-int"))
        b_live_a_freq_interval(setting);
    if (vm.count("b-live-a-freq-con"))
        b_live_a_freq_concrete(setting);

    return 0;
}
