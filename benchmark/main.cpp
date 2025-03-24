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
#include "gear_controller_test.h"
#include "gear_controller_newgear_prop.h"

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
    interval_t latency = {0, 0}, latency_i = {0,0};
    symb_time_t actual_latency_o = 0, actual_latency_i = 0;
    int error = 0;
    symb_time_t jitter = 0;
    std::vector<std::string> div_alphabet = {};
    int uncertainty_val = 0;
};

std::time_t SEED;


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

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << "\nMonitored " << (setting.trace_bound) << " events\n";
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

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << "\nMonitored " << (setting.trace_bound) << " events\n";
    return;
}

void b_live_a_freq_test(benchmark_setting& setting) {
    TA pos = Parser::parse_data(b_live_a_freq_model, "positive");
    TA neg = Parser::parse_data(b_live_a_freq_model, "negative");

    if (setting.div_alphabet.size() > 0) {
        auto div = TA::time_divergence_ta(setting.div_alphabet, true);
        pos.intersection(div);
        neg.intersection(div);
    }

    settings_t monitor_setting(setting.inclusion, setting.inclusion, setting.latency, setting.jitter);
    monitor_setting.latency_i = setting.latency_i;
    monitor_setting.jitter_i = setting.jitter;

    Testing_monitor monitor(pos, neg, monitor_setting);
   
    int max_states = 0, tmp;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t2);

    symb_time_t global = 0, t = 0;

    for (int i = 0; i < setting.trace_bound-1; i += 2) {
        if (monitor.status() != INCONCLUSIVE) {
            std::cout << "VERDICT GIVEN\n";
            break;
        }

        interval_input e({t, t}, "a");
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input(e);
        t2 = std::chrono::high_resolution_clock::now();
        
        res_tmp = ms_int.count();
        ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;


        t += setting.actual_latency_i + setting.actual_latency_o;

        interval_input e2({t, t}, "b");
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input(e2);
        t2 = std::chrono::high_resolution_clock::now();
        
        res_tmp = ms_int.count();
        ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;
    }

    t1 = std::chrono::high_resolution_clock::now();
    monitor.input({{30+setting.trace_bound,30+setting.trace_bound}, "b"});
    t2 = std::chrono::high_resolution_clock::now();
   
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << "\nMonitored " << (setting.trace_bound) << " events\n";
    return;
}


void run_gearcontroller(benchmark_setting& setting) {
    settings_t monitor_setting{setting.inclusion, setting.inclusion, setting.latency, setting.jitter};


    TA CloseClutch = Parser::parse_data(gear_controller_properties, "CloseClutch");
    TA OpenClutch = Parser::parse_data(gear_controller_properties, "OpenClutch");
    TA ReqSet = Parser::parse_data(gear_controller_properties, "ReqSet");
    TA ReqNeu = Parser::parse_data(gear_controller_properties, "ReqNeu");
    TA SpeedSet = Parser::parse_data(gear_controller_properties, "SpeedSet");
    TA test1 = Parser::parse_data(gear_controller_properties, "test1");
    TA NotCloseClutch = Parser::parse_data(gear_controller_properties, "NotCloseClutch");
    TA NotOpenClutch = Parser::parse_data(gear_controller_properties, "NotOpenClutch");
    TA NotReqSet = Parser::parse_data(gear_controller_properties, "NotReqSet");
    TA NotReqNeu = Parser::parse_data(gear_controller_properties, "NotReqNeu");
    TA NotSpeedSet = Parser::parse_data(gear_controller_properties, "NotSpeedSet");
    TA Nottest1 = Parser::parse_data(gear_controller_properties, "Nottest1");

    if (setting.div_alphabet.size() > 0) {
        auto div = TA::time_divergence_ta(setting.div_alphabet, true);
        CloseClutch.intersection(div);
        OpenClutch.intersection(div);
        ReqSet.intersection(div);
        ReqNeu.intersection(div);
        SpeedSet.intersection(div);
        test1.intersection(div);
        NotCloseClutch.intersection(div);
        NotOpenClutch.intersection(div);
        NotReqSet.intersection(div);
        NotReqNeu.intersection(div);
        NotSpeedSet.intersection(div);
        Nottest1.intersection(div);
    }

    std::vector<Delay_monitor> monitors = {
        Delay_monitor(CloseClutch, NotCloseClutch, monitor_setting),
        Delay_monitor(OpenClutch, NotOpenClutch, monitor_setting),
        Delay_monitor(ReqSet, NotReqSet, monitor_setting),
        Delay_monitor(ReqNeu, NotReqNeu, monitor_setting),
        Delay_monitor(SpeedSet, NotSpeedSet, monitor_setting),
        Delay_monitor(test1, Nottest1, monitor_setting)
    };

    auto size = 6;
    bool is_firm = false;
    int event_counter = 0;

    int tmp = 0;
    int max_states = 0;
    int max_response_time = 0;
    int response_time = 0;
    int time_horizon = 0;

    std::stringstream input_stream(gear_controller_input, std::ios::in);

    std::vector<timed_input_t> events;
    std::vector<timed_input_t> tmpevents;

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);

    while ((not is_firm) && (setting.trace_bound == 0 || event_counter < setting.trace_bound)) {

        
        tmpevents = EventParser::parse_input(&input_stream, 1);
        events.clear();

        if (tmpevents.size() == 0) break;
        for (auto &e : tmpevents) {
            events.push_back(timed_input_t({e.time.first - setting.uncertainty_val, e.time.second + setting.uncertainty_val}, e.label, e.type));
        }
        
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
    for (int i = 0; i < size; ++i) {
        std::cout << monitors[i].status() << ", ";
    }
    std::cout << '\n';
}


void run_gearcontroller_testing(benchmark_setting& setting, bool testing) {
    settings_t monitor_setting{setting.inclusion, setting.inclusion, setting.latency, setting.jitter};
    
    monitor_setting.latency_i = setting.latency_i;
    monitor_setting.jitter_i = setting.jitter;

    TA pos = Parser::parse_data(gear_controller_test_model, "positive");
    TA neg = Parser::parse_data(gear_controller_test_model, "negative");

    if (setting.div_alphabet.size() > 0) {
        auto div = TA::time_divergence_ta({"ReqNewGear", "NewGear"}, true);
        pos.intersection(div);
        neg.intersection(div);
    }

    auto monitor_test = Testing_monitor(pos, neg, monitor_setting);
    auto monitor = Delay_monitor(pos, neg, monitor_setting);


    int event_counter = 0;

    int tmp = 0;
    int max_states = 0;
    int max_response_time = 0;
    int response_time = 0;
    int time_horizon = 0;

    std::stringstream input_stream(gear_controller_test_input, std::ios::in);

    std::vector<timed_input_t> events;
    std::vector<timed_input_t> tmpevents;

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);


    bool input_mode = true;

    std::cout << "Seed: " << SEED;
    srand (SEED);
    int error = setting.error;
    int error_count = 0;
    bool error_b = false;

    symb_time_t jitter = 0, global_t = 0, t = 0;
    
    // if (testing)
    //     std::cout << "\nIndex\tPos-in-lat\tPos-out-lat\tPos-diff\tNeg-in-lat\tNeg-out-lat\tNeg-diff\tObservation\ttimestamp\tErrors";
    // else
    //     std::cout << "\nIndex\tPos-out-lat\tNeg-out-lat\tObservation\ttimestamp\tErrors";

    while ((monitor.status() == INCONCLUSIVE) && (monitor_test.status() == INCONCLUSIVE) && (setting.trace_bound == 0 || event_counter < setting.trace_bound)) {
        
        if (testing) {
            t = global_t + 50;
            
            events.clear();
            events.push_back(timed_input_t({t - setting.uncertainty_val, t + setting.uncertainty_val}, "ReqNewGear"));
            
            t += + 150 - error + rand() % (1056 + 2 * error);
            
            if (t > global_t + 1255 || t < global_t + 200) {
            error_b = true;
            error_count++;
            }


            jitter = rand() % (setting.jitter + 1);
            t += jitter;
            jitter = rand() % (setting.jitter + 1);
            t += jitter;
            t += setting.actual_latency_o + setting.actual_latency_i;

            events.push_back(timed_input_t({t - setting.uncertainty_val, t + setting.uncertainty_val}, "NewGear"));
        } else {
            t = global_t + setting.latency.second + setting.jitter;

            jitter = rand() % (setting.jitter + 1);

            events.clear();
            events.push_back(timed_input_t({t - setting.uncertainty_val + setting.actual_latency_o + jitter, t + setting.uncertainty_val + setting.actual_latency_o + jitter}, "ReqNewGear"));

            global_t = t;

            t += + 150 - error + rand() % (1056 + 2 * error);
            
            if (t - global_t > 1205 || t - global_t < 150) {
                error_b = true;
                error_count++;
            }

            jitter = rand() % (setting.jitter + 1);
        
            events.push_back(timed_input_t({t - setting.uncertainty_val + setting.actual_latency_o + jitter, t + setting.uncertainty_val + setting.actual_latency_o + jitter}, "NewGear"));
        }

        // assert(t - (global_t + 50) <= 1205 + setting.actual_latency_i + setting.actual_latency_o + 2 * setting.jitter + error);

        global_t = t;
        
        for (const auto& e : events) {
            
            if (testing) {
                t1 = std::chrono::high_resolution_clock::now();
                monitor_test.input(e);
                t2 = std::chrono::high_resolution_clock::now();
            } else {
                t1 = std::chrono::high_resolution_clock::now();
                monitor.input(e);
                t2 = std::chrono::high_resolution_clock::now();
            }

            
            tmp = time.count();
            time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
            response_time = time.count() - tmp;
            max_response_time = response_time > max_response_time ? response_time : max_response_time;

            tmp = max_states;
            max_states = 0;

            if (testing)
                max_states = monitor_test.positive_state_estimate().size() + monitor_test.negative_state_estimate().size();
            else
                max_states = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();

            event_counter += 1;

            max_states = tmp > max_states ? tmp : max_states;
            
            time_horizon = events[events.size()-1].time.second;



            // monitor.print_status(std::cout);


            auto pos_in_lat = boost::icl::interval_set<symb_time_t>(),
                pos_out_lat = boost::icl::interval_set<symb_time_t>(),
                neg_in_lat = boost::icl::interval_set<symb_time_t>(),
                neg_out_lat = boost::icl::interval_set<symb_time_t>(),
                pos_in_out = boost::icl::interval_set<symb_time_t>(),
                neg_in_out = boost::icl::interval_set<symb_time_t>();

            if (testing) {
                for (const auto& s : monitor_test.positive_state_estimate()) {
                    pos_in_lat += s.get_input_latency();
                    pos_in_out += s.get_input_output_interval();
                    pos_out_lat += s.get_output_latency();
                }

                for (const auto& s : monitor_test.negative_state_estimate()) {
                    neg_in_lat += s.get_input_latency();
                    neg_in_out += s.get_input_output_interval();
                    neg_out_lat += s.get_output_latency();
                }
            } else {
                for (const auto& s : monitor.positive_state_estimate())
                    pos_out_lat += s.get_latency();

                for (const auto& s : monitor.negative_state_estimate())
                    neg_out_lat += s.get_latency();
            }

            // if (testing)
            //     std::cout << "\n" << event_counter << '\t' << pos_in_lat << '\t' << pos_out_lat <<  '\t' << pos_in_out << '\t' << neg_in_lat << '\t' << neg_out_lat << '\t' << neg_in_out << '\t' << e.label << '\t' << "[" << e.time.first << ", " << e.time.second << "]\t";
            // else
            //     std::cout << "\n" << event_counter << '\t' << pos_out_lat <<  '\t' << neg_out_lat << '\t' << e.label << '\t' << "[" << e.time.first << ", " << e.time.second << "]\t";

            // std::cout << "Positive reach set:\n";
            // for (auto e : monitor.positive_state_estimate()) {
            //     e.print(std::cout, pos);
            // }
        }
        // if (error_b) {
        //     std::cout << "Error"; 
        //     error_b = false;
        // }
    }
    std::cout << event_counter << ',' << max_response_time << ',' << max_states << '\n';
    // std::cout << "\nMonitored " << event_counter << " events in " << time.count() << "ns\nVerdict: " << monitor.status() <<
    //              "\nMax states: "<< max_states << "\nmax response: "<< max_response_time << "ns\nTime Horizon: " << time_horizon <<"\nMemory: " << sizeof(monitor) <<'\n';
    // std::cout << "Error count: " << error_count << '\n';
}


std::vector<timed_input_t> simulate_gear_controller_classic(const benchmark_setting& setting, int& current_gear, symb_time_t& current_time) {
    std::vector<timed_input_t> inputs;
    
    srand (SEED);

    int usecase = 0;
    bool fromN = current_gear == 1 ? true : false;

    char from_label,to_label;
    std::string label = "ReqNewGear";

    switch (current_gear) {
        case 0: from_label = '0'; break;
        case 1: from_label = '1'; break;
        case 2: from_label = '2'; break;
        case 3: from_label = '3'; break;
        case 4: from_label = '4'; break;
        case 5: from_label = '5'; break;
        case 6: from_label = '6'; break;
        default: from_label = '7';
    }

    // 0 is reverse, 1 is neutral
    if (current_gear == 0)
        ++current_gear;
    else if (current_gear == 7)
        --current_gear;
    else if (rand() % 2)
        ++current_gear;
    else
        --current_gear;

    bool toN = current_gear == 1 ? true : false;
    
    current_time += setting.error;
    

    switch (current_gear) {
        case 0: to_label = '0'; break;
        case 1: to_label = '1'; break;
        case 2: to_label = '2'; break;
        case 3: to_label = '3'; break;
        case 4: to_label = '4'; break;
        case 5: to_label = '5'; break;
        case 6: to_label = '6'; break;
        default: to_label = '7';
    }

    label = label + from_label + to_label;

    inputs.push_back({{current_time - setting.uncertainty_val, current_time + setting.uncertainty_val},label});

    int newgeartime = current_time;

    /*  if fromN, then usecase1 is not possible.
        if usecase1 OR toN, then usecase2 is not possible.*/

    bool usecase1 = !fromN && (rand() % 5) > 1;
    bool usecase2 = !toN && (rand() % 3) > 1;
    bool neutral = toN || fromN;

    if (usecase1) {
        inputs.push_back({{current_time + 250 - setting.uncertainty_val, current_time + 250 + setting.uncertainty_val}, "UseCase1"});
        int t = 0;

        if (neutral) t = 550;
        else t = 700;

       //  700 + [0, 1055 - 700]

        current_time += t - setting.error + (rand() % (1055 - t + 2 * setting.error));

        inputs.push_back({{current_time - setting.uncertainty_val, current_time + setting.uncertainty_val}, "NewGear"});
    } else if (usecase2) {
        int t = 0;
        
        if (neutral) t = 150;
        else t = 400;

        inputs.push_back({{current_time + t - setting.uncertainty_val, current_time + t + setting.uncertainty_val}, "UseCase2"});

        if (neutral) t = 450;
        else t = 750;

        current_time += t - setting.error + (rand() % (1206 - t + 2*setting.error));

        inputs.push_back({{current_time - setting.uncertainty_val, current_time + setting.uncertainty_val}, "NewGear"});
    } else {
        int t = 0;

        if (neutral) t = 150;
        else t = 400;

        current_time += t - setting.error + (rand() % (901 - t + 2*setting.error));

        inputs.push_back({{current_time - setting.uncertainty_val, current_time + setting.uncertainty_val}, "NewGear"});
    }


    return inputs;
}

void gear_controller_sim_bench(const benchmark_setting& setting) {
    auto prop = gear_control_newgear_prop();

    auto labels = prop.first.labels();

    if (setting.div_alphabet.size() > 0) {
        TA div = TA::time_divergence_ta(std::vector<label_t>(labels.begin(), labels.end()), true);
        prop.first.intersection(div);
        prop.second.intersection(div);
    }

    settings_t monitor_setting(setting.inclusion, setting.inclusion, setting.latency, setting.jitter);

    Interval_monitor monitor(prop.first, prop.second, monitor_setting);

    // std::cout << "Positive\n";
    // for (const auto& s : monitor.positive_state_estimate()) {
    //     s.print(std::cout, prop.first);
    // }
    // std::cout << "Negative\n";
    // for (const auto& s : monitor.negative_state_estimate()) {
    //     s.print(std::cout, prop.second);
    // }

    int current_gear = 1;
    symb_time_t current_time = 0;
    int count = 0;

    int tmp, response_time, max_response_time = 0, max_states = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);

    while(monitor.status() == INCONCLUSIVE && count < setting.trace_bound) {
        auto input = simulate_gear_controller_classic(setting, current_gear, current_time);

        for (const auto& e : input) {
            
            // std::cout << "\n\n" << e.label << ", " << e.time.first << "\n\n";
            
            t1 = std::chrono::high_resolution_clock::now();
            monitor.input(e);
            t2 = std::chrono::high_resolution_clock::now();
            
            // std::cout << "Positive\n";
            // for (const auto& s : monitor.positive_state_estimate()) {
                // s.print(std::cout, prop.first);
            // }
            // std::cout << "Negative\n";
            // for (const auto& s : monitor.negative_state_estimate()) {
                // s.print(std::cout, prop.second);
            // }
            tmp = time.count();
            time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
            response_time = time.count() - tmp;
            max_response_time = response_time > max_response_time ? response_time : max_response_time;
            
            tmp = monitor.negative_state_estimate().size() + monitor.positive_state_estimate().size();
            max_states = tmp > max_states ? tmp : max_states;

            if (++count >= setting.trace_bound)
                break;
        }
    }

    std::cout << count << ',' << time.count() << ',' << max_response_time << ',' << max_states << ',' << monitor.status() << '\n'; 

    // std::cout << "Monitored " << count << " events in " << time.count() << 
                //  "ns\nVerdict: " << monitor.status() << "\nMax states: "<< max_states << "\nmax response: "<< max_response_time << "ns\nMemory: " << sizeof(monitor) << '\n';
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-benchmark --pos <name> <path> --neg <name> <path> --input <path>")
            ("length", po::value<int>()->default_value(0, "0"), "Bound on the number of observations. 0 means no bound")
            ("inclusion", "Enable inclusion and inactive clock abstraction")
            ("uncertainty", po::value<symb_time_t>()->default_value(0, "0"), "Random timing uncertainty added to observations")
            ("div,d", po::value<std::vector<std::string>>()->multitoken()->default_value({}, "Empty"), "<list of labels> : Take time divergence into account.")
            ("latency", po::value<std::vector<symb_time_t>>()->multitoken()->default_value({0, 0}, "0 0"), "Specify latency upper and lower bound parameters")
            ("actual-latency", po::value<symb_time_t>()->default_value(0, "0"), "Latency used to shift outputs")
            ("actual-latency-i", po::value<symb_time_t>()->default_value(0, "0"), "Latency used to shift inputs")
            ("jitter", po::value<symb_time_t>()->default_value(0, "0"), "Specify the jitter upper bound parameter")
            ("gear-controller", "Run Gear controller benchmark")
            ("gear-controller-input", "Run Gear controller test with inputs and outputs")
            ("gear-controller-output", "Run Gear controller test with only outputs")
            ("error", po::value<int>()->default_value(0, "0"), "Error margin of the test")
            ("input-latency", po::value<std::vector<symb_time_t>>()->multitoken()->default_value({0,0}, "0 0"), "Latency lower and upper bound for input channel")
            ("b-live-a-freq-int","Run the b-liveness & a-frequency benchmark with interval time")
            ("seed", po::value<std::time_t>(), "Provide a seed for randomness, default is time")
            ("b-live-a-freq-con","Run the b-liveness & a-frequency benchmark with concrete time")
            ("b-live-a-freq-test","Run the b-liveness & a-frequency benchmark under test")
            ("gear-controller-sim-bench", "Monitor a complex property over a simulation of the gear controller");

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
    auto latency_i = vm["input-latency"].as<std::vector<symb_time_t>>(); 
    
    if (latency.size() != 2 && latency_i.size() != 2) {
        std::cerr << "Error: Latency needs exactly two arguments (lower and upper bound)\n";
        exit(-1);
    }


    benchmark_setting setting;
    setting.trace_bound = vm["length"].as<int>();
    setting.jitter = vm["jitter"].as<symb_time_t>();
    setting.inclusion = vm.count("inclusion");
    setting.latency = {latency[0], latency[1]};
    setting.actual_latency_o = vm["actual-latency"].as<symb_time_t>();
    setting.actual_latency_i = vm["actual-latency-i"].as<symb_time_t>();
    setting.div_alphabet = vm["div"].as<std::vector<std::string>>();
    setting.uncertainty_val = vm["uncertainty"].as<symb_time_t>();
    setting.latency_i = {latency_i[0], latency_i[1]};
    setting.error = vm["error"].as<int>();
    
    if (vm.count("seed"))
        SEED = vm["seed"].as<std::time_t>();
    else
        SEED = std::time(NULL);
    
    if (vm.count("gear-controller"))
        run_gearcontroller(setting);
    if (vm.count("gear-controller-input"))
        run_gearcontroller_testing(setting, true);
    if (vm.count("gear-controller-output"))
        run_gearcontroller_testing(setting, false);
    if (vm.count("b-live-a-freq-int"))
        b_live_a_freq_interval(setting);
    if (vm.count("b-live-a-freq-con"))
        b_live_a_freq_concrete(setting);
    if (vm.count("b-live-a-freq-test"))
        b_live_a_freq_test(setting);
    if (vm.count("gear-controller-sim-bench"))
        gear_controller_sim_bench(setting);

    return 0;
}
