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
#include "errors.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

#include <map>
#include <time.h>
#include <stdlib.h>
#include <random>

namespace po = boost::program_options;
using namespace monitaal;
struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};
struct bin_settings_t
{
    bool verbose = false;
    bool silent = false;
    uint32_t event_counter = 0;
    TA positive, negative;

    bin_settings_t(const TA& positive, const TA& negative) : positive(positive), negative(negative) {};
};

void run_benchmark_concrete(Concrete_monitor& monitor, bin_settings_t& settings, int limit, bool advance) {
    int max_states = 0, tmp = 0;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t2);

    for (int i = 0; i < limit-1; ++i) {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input(concrete_input(i, "a"));
        t2 = std::chrono::high_resolution_clock::now();

        res_tmp = ms_int.count();
        ms_int += std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;
    }
    t1 = std::chrono::high_resolution_clock::now();
    monitor.input({11+limit, "b"});
    t2 = std::chrono::high_resolution_clock::now();
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ms\nMax response time: " << max_response << " ms\nMax states: " << max_states << '\n';
    return;
}

void run_benchmark_interval(Interval_monitor& monitor, bin_settings_t& settings, int limit, bool overlap) {
    int max_states = 0, tmp;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t2);

    for (int i = 0; i < limit-1; ++i) {
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
        ms_int += std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        res_tmp = ms_int.count() - res_tmp;
        max_response = res_tmp > max_response ? res_tmp : max_response;

        tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
        max_states = tmp > max_states ? tmp : max_states;
    }
    if (overlap) {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input({{30+limit,30+limit}, "b"});
        t2 = std::chrono::high_resolution_clock::now();
    } else {
        t1 = std::chrono::high_resolution_clock::now();
        monitor.input({{30,30}, "b"});
        t2 = std::chrono::high_resolution_clock::now();
    }
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ms\nMax response time: " << max_response << " ms\nMax states: " << max_states << '\n';
    return;
}

template<class state_t>
void interactive_monitoring(Monitor<state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& in) {
    std::string input = "";

    std::vector<timed_input_t> events;

    if (monitor.status() == INCONCLUSIVE)
        out << "Interactive monitor (respond with \"q\" to quit)\n";

    while (monitor.status() == INCONCLUSIVE) {
        if (settings.verbose && !settings.silent) {
            out << "\nPositive state estimate:\n";
            for (const auto& s : monitor.positive_state_estimate()) {
                s.print(out, settings.positive);
            }

            out << "\nNegative state estimate:\n";
            for (const auto& s : monitor.negative_state_estimate()) {
                s.print(out, settings.negative);
            }
            out << '\n';
        }

        while (true) {
            out << "Next event: ";
            std::getline(in, input);
            if (input == "q") break;

            membuf sbuf(input.data(), input.data() + input.size());
            auto in = std::istream(&sbuf);

            try
            {
                events = EventParser::parse_input(&in, 0);
            }
            catch(const base_error& e)
            {
                out << e.what() << '\n';
                continue;
            }
            break;
        }
        if (input == "q") break;

        monitor.input(events);
        settings.event_counter += events.size();
    }
}

template void interactive_monitoring<concrete_state_t>(Monitor<concrete_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& in);
template void interactive_monitoring<symbolic_state_t>(Monitor<symbolic_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& in);
template void interactive_monitoring<delay_state_t>(Monitor<delay_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& in);

template <class state_t>
void monitor_from_file(Monitor<state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& input) {
    std::vector<timed_input_t> events;

    while (not input.eof() || monitor.status() == INCONCLUSIVE) {        
        events = EventParser::parse_input(&input, 1000);
        monitor.input(events);
        settings.event_counter += events.size();
    }
}

template void monitor_from_file<concrete_state_t>(Monitor<concrete_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& input);
template void monitor_from_file<symbolic_state_t>(Monitor<symbolic_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& input);
template void monitor_from_file<delay_state_t>(Monitor<delay_state_t>& monitor, bin_settings_t& settings, std::ostream& out, std::istream& input);

bool arg_type(const po::variables_map& vm) {
    if (vm.count("type")) {
        std::string type = vm["type"].as<std::string>();
        if (type == "interval")
            return true;
        else if (type == "concrete")
            return false;
        else {
            std::cerr << "Error: type must be concrete or interval\n";
            exit(-1);
        }
    }
    return false;
}

void print_dot(const TA& pos, const TA& neg, std::ostream& out) {
    pos.print_dot(out);
    out << '\n';
    neg.print_dot(out);
    out << '\n';
}


void production_line_ta(int size, int lower_bound, int upper_bound, int goal, int jitter, int unobservables, int unobs_clusters, int reps, bool with_assumption) {

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2);


    t1 = std::chrono::high_resolution_clock::now();
    clock_map_t clocks, prop_clocks;
    locations_t locations, pos_locations, neg_locations;
    edges_t edges, pos_edges, neg_edges;

    clocks.insert({{0, "0"}, {1, "x"}});

    locations.push_back({true, 0, "l_0", {}});

    for (int i = 1; i <= size; ++i) {
        locations.push_back({true, i, "l_" + std::to_string(i), {}});
        edges.push_back(edge_t(i-1, i, {constraint_t::lower_non_strict(1, lower_bound), constraint_t::upper_non_strict(1, upper_bound)}, {1}, "a_" + std::to_string(i)));
        // edges.push_back(edge_t(size, size, {}, {}, "a_" + std::to_string(i)));
    }
    edges.push_back(edge_t(size, size, {}, {}, "a_" + std::to_string(size)));

    TA assum("Assumption", clocks, locations, edges, 0);


    prop_clocks.insert({{0, "0"}, {1, "y"}});

    pos_locations.push_back(location_t(true, 0, "l_0", {}));
    pos_locations.push_back(location_t(false, 1, "l_1", {}));
    pos_edges.push_back(edge_t(0, 1, {}, {1}, "a_1"));
    pos_edges.push_back(edge_t(1, 0, {constraint_t::upper_non_strict(1, goal)}, {}, "a_" + std::to_string(size)));
    pos_edges.push_back(edge_t(0, 0, {}, {}, "a_" + std::to_string(size)));

    TA pos("positive", prop_clocks, pos_locations, pos_edges, 0);

    neg_locations.push_back(location_t(false, 0, "l_0", {}));
    neg_locations.push_back(location_t(false, 1, "l_1", {}));
    neg_locations.push_back(location_t(true, 2, "l_2", {}));
    neg_edges.push_back(edge_t(0, 0, {}, {}, "a_" + std::to_string(size)));
    neg_edges.push_back(edge_t(0, 1, {}, {1}, "a_1"));
    neg_edges.push_back(edge_t(1, 0, {constraint_t::upper_non_strict(1, goal)}, {}, "a_" + std::to_string(size)));
    neg_edges.push_back(edge_t(1, 2, {constraint_t::lower_strict(1, goal)}, {}, "a_" + std::to_string(size)));
    neg_edges.push_back(edge_t(2, 2, {}, {}, "a_" + std::to_string(size)));
    neg_edges.push_back(edge_t(2, 2, {}, {}, "a_1"));

    TA neg("negative", prop_clocks, neg_locations, neg_edges, 0);

    t1 = std::chrono::high_resolution_clock::now();

    if (with_assumption) {
        pos.intersection(assum);
        neg.intersection(assum);
    }

    std::random_device r;
    std::default_random_engine eng (r());
    std::uniform_int_distribution<int> obs_dist(lower_bound, upper_bound);
    std::uniform_int_distribution<int> jit_dist(-jitter, jitter);

    std::vector<timed_input_t> word;

    t2 = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);

    std::cout << "Built automata in: " << time.count() << "ns\n";

    time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t2);


    settings_t setting;
    setting.inclusion = false;

    std::vector<std::vector<int>> results(size, std::vector<int>(4, 0));

    int max_response_time = 0;
    int avg_response_time = 0;


    for (int l = 0; l < reps; ++l) {
        int tmp = 0;
        int response_time = 0;
        int global_time = 0;
        int unobs_count = 0,
            unobs_cluster_count = 0,
            unobs_cluster_size = unobservables / (unobs_clusters ? unobs_clusters : 1),
            unobs_interval = (size - unobservables) / (unobs_clusters + 1);
        unobs_interval += (((size - unobservables) % (unobs_clusters + 1)) == 0) ? 0 : 1;


        monitaal::Single_monitor<symbolic_state_t> positive(pos, setting);
        monitaal::Single_monitor<symbolic_state_t> negative(neg, setting);
        monitaal::Single_monitor<symbolic_state_t> assumption(assum, setting);


        for (int i = 1; i < size+1; ++i) {
            interval_t interval;
            label_t label = "a_" + std::to_string(i);
            input_type_e type;
            
            int obs_time = obs_dist(eng) + global_time;
            global_time = obs_time; 
            obs_time += jit_dist(eng);

            if (i >= unobs_cluster_count * (unobs_interval + unobs_cluster_size) && 
                i <= (unobs_cluster_count + 1) * unobs_interval + unobs_cluster_count * unobs_cluster_size) {

                interval = interval_t(obs_time - jitter, obs_time + jitter);
                

                type = ONCE;
            }
            else {
                interval = interval_t(0, (size -1) * upper_bound);
                type = OPTIONAL;
                ++unobs_count;
                if (unobs_count % unobs_cluster_size == 0) {
                    ++unobs_cluster_count;
                }
            }

            timed_input_t input(interval, label, type);

            t1 = std::chrono::high_resolution_clock::now();
            if (with_assumption)
                assumption.input(input);
            positive.input(input);
            negative.input(input);
            t2 = std::chrono::high_resolution_clock::now();

            tmp = time.count();
            time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
            response_time = time.count() - tmp;
            max_response_time = response_time > max_response_time ? response_time : max_response_time;

            avg_response_time = avg_response_time + (response_time - avg_response_time)/i;

            if (assumption.status() == OUT) {
                std::cout << i << " : " << input.time.first << " " << input.type << "\n";
                ++(results[i-1][0]);
                break;
            }
            else if (positive.status() == OUT) {
                ++(results[i-1][1]);
                break;
            }
            else if (negative.status() == OUT) {
                ++(results[i-1][2]);
                break;
            }
            else {
                ++(results[i-1][3]);
            }
        }
    }

    std::cout << "Observations, violation, positive, negative, unknown\n";
    for (int obs = 0; obs < size; ++obs) {
        if (results[obs][3] == reps) continue;
        std::cout << obs+1 << ", " << results[obs][0] << ", " << results[obs][2] << ", " << results[obs][1] << ", " << results[obs][3] << "\n";
    }

    std::cout << "monitoring time: \t" << time.count() << "ns\nmax response: \t\t"<< max_response_time << "ns\nAvg. response: \t\t" << avg_response_time << "ns\n";
   

    

    // int i = 0;
    // std::string verdict = "None";
    // for (const auto &o : word) {
    //     ++i;
    //     t1 = std::chrono::high_resolution_clock::now();
    //     if (assumption.input(o) == OUT || positive.input(o) == OUT || negative.input(o) == OUT) {
    //         if (assumption.status() == OUT) {
    //             std::cout << "Verdict: Violation after a_" << i << "\n";
    //         }
    //         else if (positive.status() == OUT) {
    //             std::cout << "Verdict: Negative after a_" << i << "\n";
    //         }
    //         else if (negative.status() == OUT) {
    //             std::cout << "Verdict: Positive after a_" << i << "\n";
    //         }
    //         t2 = std::chrono::high_resolution_clock::now();
            
    //         tmp = time.count();
    //         time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
    //         response_time = time.count() - tmp;
    //         max_response_time = response_time > max_response_time ? response_time : max_response_time;
    //         break;
    //     }
    //     t2 = std::chrono::high_resolution_clock::now();

    //     tmp = time.count();
    //     time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
    //     response_time = time.count() - tmp;
    //     max_response_time = response_time > max_response_time ? response_time : max_response_time;
    // }

    // for (auto s : positive.state_estimate())
    //     s.print(std::cout, pos);

    // std::cout << "monitoring time: " << time.count() << "ns\nmax response: "<< max_response_time << "\n";
}

void assumption_experiment_01(std::string path) {
    int limit = 1000;
    
    settings_t setting;
    setting.inclusion = true;

    TA pos = Parser::parse(path.c_str(), "positive"),
       neg = Parser::parse(path.c_str(), "negative"),
       ass = Parser::parse(path.c_str(), "assumption");

    pos.intersection(ass);
    neg.intersection(ass);


    std::random_device r;
    std::default_random_engine eng (r());
    std::uniform_int_distribution<int> uniform_dist(49, 101);

    std::vector<std::string> line{"a", "b", "c", "d", "e", "f", "g"};

    int results[7][4];


    for (int i = 0; i < limit; ++i) {
        int global_time = 0;
        int obs_count = 0;
        int verdict = 0;

        monitaal::Single_monitor<concrete_state_t> positive(pos, setting);
        monitaal::Single_monitor<concrete_state_t> negative(neg, setting);
        monitaal::Single_monitor<concrete_state_t> assumption(ass, setting);
        
        for (const auto& label : line) {
            global_time += uniform_dist(eng);
            timed_input_t input(global_time, label);
            
            ++obs_count;

            if (assumption.input(input) == OUT) {
                ++(results[obs_count-1][0]);
                break;
            }
            else if (positive.input(input) == OUT) {
                ++(results[obs_count-1][1]);
                break;
            }
            else if (negative.input(input) == OUT) {
                ++(results[obs_count-1][2]);
                break;
            }
            else {
                ++(results[obs_count-1][3]);
            }
        }
    }

    std::cout << "Observations, violation, positive, negative, unknown\n";
    for (int obs = 0; obs < 7; ++obs) {
        std::cout << obs << ", " << results[obs][0] << ", " << results[obs][1] << ", " << results[obs][2] << ", " << results[obs][3] << "\n";
    }
}

void assumption_experiment_02(std::string path) {
    // int limit = 1000;
    
    // settings_t setting;
    // setting.inclusion = true;

    // TA pos = Parser::parse(path.c_str(), "positive"),
    //    neg = Parser::parse(path.c_str(), "negative"),
    //    ass = Parser::parse(path.c_str(), "assumption");

    // pos.intersection(ass);
    // neg.intersection(ass);


    // std::random_device r;
    // std::default_random_engine eng (r());
    // std::uniform_int_distribution<int> uniform_dist(7, 10);

    // std::map<int, int> results;
    
    // for (int i = 0; i < limit; ++i) {
    //     int global_time = 0,
    //         event_count = 0;
    //     monitaal::Single_monitor<symbolic_state_t> positive(pos, setting);
    //     monitaal::Single_monitor<symbolic_state_t> negative(neg, setting);
    //     monitaal::Single_monitor<symbolic_state_t> assumption(ass, setting);

    //     while (assumption.status() == ACTIVE && 
    //         positive.status() == ACTIVE && 
    //         negative.status() == ACTIVE) 
    //     {
    //         int t = uniform_dist(eng);

    //         std::vector<timed_input_t> inputs{{global_time, "fault", OPTIONAL},
    //                     {++global_time, "start"},
    //                     {{global_time, global_time + t + 1}, "fault", OPTIONAL},
    //                     {{global_time + t - 1, global_time + t + 1}, "stop"},
    //                     {global_time + t + 1, "move"}};
            
    //         global_time += t + 1;

    //         bool exit = false;

    //         for (const auto& i : inputs) {
    //             // std::cout << "Events: " << event_count << "\n";
                
    //             exit |= assumption.input(i) == OUT;
    //             exit |= positive.input(i) == OUT;
    //             exit |= negative.input(i) == OUT;
    //             // if (assumption.status() == OUT)
    //             //     std::cout << "Assumption OUT!\n";
    //             // if (positive.status() == OUT)
    //             //     std::cout << "Positive OUT!\n";
    //             // if (negative.status() == OUT)
    //             //     std::cout << "Negative OUT!\n";
    //             if (exit) break;
    //         }
    //         ++event_count;
    //     }
    //     if (results.contains(event_count))
    //         results[event_count] += 1;
    //     else 
    //         results.insert({event_count, 1});
    // }

    
    // for (const auto& [e, i] : results) {
    //     std::cout << e << "\t" << i << "\n";
    // }

    
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-bin --pos <name> <path> --neg <name> <path>")
            ("pos,p", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> : Property automaton.")
            ("neg,n", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> : Negated property automaton.")
            ("type,t", po::value<std::string>(), "Input type (concrete or interval) default = concrete.")
            ("input,i", po::value<std::string>(), "Monitor events contained in file.")
            ("inclusion,u", "Enable inclusion checking for duplicate states")
            ("verbose,v", "Prints more information on the monitoring procedure.")
            ("silent,s", "removes all outputs")
            ("print-dot,o", "Prints the dot graphs of the given automata.")
            ("benchmark", po::value<int>(), "Run predefined benchmark")
            ("advance,a", "For bencharking. Make sure that time points advance")
            ("div,d", po::value<std::vector<std::string>>()->multitoken(), "<list of labels> : Take time divergence into account.")
            ("assumption1", po::value<std::string>(), "Path to assumption model")
            ("assumption2", po::value<std::string>(), "Path to assumption model")
            ("prod-line", po::value<std::vector<int>>()->multitoken(), "size, lower, upper, goal, jitter, #unobs, #unobs clusters, repetitions")
            ("with-assumption", "With or without assumption");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

    if (vm.count("help")) {
        std::cout << options << std::endl;
        return 1;
    }

    if (vm.count("assumption1")) {
        std::string assumptionarg = vm["assumption1"].as<std::string>();
        assumption_experiment_01(assumptionarg);
        return 0;
    }
    if (vm.count("assumption2")) {
        std::string assumptionarg = vm["assumption2"].as<std::string>();
        assumption_experiment_02(assumptionarg);
        return 0;
    }
    if(vm.count("prod-line")) {
        std::vector<int> par = vm["prod-line"].as<std::vector<int>>();
        production_line_ta(par[0], par[1], par[2], par[3], par[4], par[5], par[6], par[7], vm.count("with-assumption"));
        return 0;
    }

    // Validate arguments
    try {
        po::notify(vm);
    } catch (boost::wrapexcept<po::required_option>& e) {
        std::cerr << "Error: Some required fields not given\n";
        exit(-1);
    }

    auto posarg = vm["pos"].as<std::vector<std::string>>();
    auto negarg = vm["neg"].as<std::vector<std::string>>();

    if (posarg.size() != 2 || negarg.size() != 2) {
        for (int i = 0; i < posarg.size(); ++i)
            std::cout << posarg[i];
        std::cerr << "Error: --pos and --neg args require two arguments\n";
        exit(-1);
    }

    TA pos = Parser::parse(&posarg[1][0], &posarg[0][0]);
    TA neg = Parser::parse(&negarg[1][0], &negarg[0][0]);

    bool is_interval = arg_type(vm);

    if (vm.count("print-dot")) {
        std::cout << "DOT GRAPHS\n";
        print_dot(pos, neg, std::cout);
    }

    // Handle time divergence
    if (vm.count("div")) {
        auto alphabet = vm["div"].as<std::vector<std::string>>();
        TA div = TA::time_divergence_ta(alphabet, true);
        pos.intersection(div);
        neg.intersection(div);

        if (vm.count("print-dot")) {
            std::cout << "DOT GRAPHS with time divergence conjunction\n";
            print_dot(pos, neg, std::cout);
        }

    }

    bin_settings_t settings(pos, neg);
    settings.verbose = vm.count("verbose") > 0;
    settings.silent = vm.count("silent") > 0;

    settings_t mon_setting = settings_t();
    mon_setting.inclusion = vm.count("inclusion");

    Interval_monitor monitor_int(pos, neg, mon_setting);
    Concrete_monitor monitor_con(pos, neg, mon_setting);

    if (vm.count("benchmark")) {
        if (is_interval) {
            run_benchmark_interval(monitor_int, settings, vm["benchmark"].as<int>(), vm.count("advance"));
            if (!settings.silent)
                std::cout << "Monitoring ended, verdict is: " << monitor_int.status() << "\nMonitored " << (vm["benchmark"].as<int>()) << " events\n";

        }
        else {
            run_benchmark_concrete(monitor_con, settings, vm["benchmark"].as<int>(), vm.count("advance"));
            if (!settings.silent)
                std::cout << "Monitoring ended, verdict is: " << monitor_con.status() << "\nMonitored " << (vm["benchmark"].as<int>()) << " events\n";

        }
        return 0;
    }

    // Monitoring events from file
    if (vm.count("input")) {
        auto inputarg = vm["input"].as<std::string>();
        
        std::filebuf fb;
        fb.open(inputarg, std::ios::in);
        std::istream stream(&fb);

        if (is_interval)
            monitor_from_file<symbolic_state_t>(monitor_int, settings, std::cout, stream);
        else
            monitor_from_file<concrete_state_t>(monitor_con, settings, std::cout, stream);

        fb.close();
    }
    else {
        // Interactive Monitoring
        if (is_interval)
            interactive_monitoring<symbolic_state_t>(monitor_int, settings, std::cout, std::cin);
        else
            interactive_monitoring<concrete_state_t>(monitor_con, settings, std::cout, std::cin);
    }


    if (is_interval) {
        if (!settings.silent)
            std::cout << "Monitoring ended, verdict is: " << monitor_int.status() << "\nMonitored " << settings.event_counter << " events\n";
        return monitor_int.status() == INCONCLUSIVE;

    }

    if (!settings.silent)
        std::cout << "Monitoring ended, verdict is: " << monitor_con.status() << "\nMonitored " << settings.event_counter << " events\n";
    return monitor_con.status() == INCONCLUSIVE;
    /*
    std::string input = "";
    
    if (monitor.status() != INCONCLUSIVE)
        std::cout << "Interactive monitor (respond with \"q\" to quit)\n";

    while (monitor.status() == INCONCLUSIVE || input == "q\n") {
        if (verbose) {
            std::cout << "\nPositive state estimate:\n";
            for (const auto& s : monitor.positive_state_estimate()) {
                std::cout << '\t';
                s.print(std::cout, pos);
            }

            std::cout << "\nNegative state estimate:\n";
            for (const auto& s : monitor.negative_state_estimate()) {
                std::cout << '\t';
                s.print(std::cout, neg);
            }
            std::cout << '\n';
        }
        std::cout << "Next event: ";
        std::getline(std::cin, input);
        
        membuf sbuf(input.data(), input.data() + input.size());
        auto in = std::istream(&sbuf);

        try
        {
            concrete_events = EventParser::parse_concrete_input(&in);
        }
        catch(const base_error& e)
        {
            std::cout << e.what() << '\n';
            continue;
        }

        monitor.input(concrete_events);
        event_counter += concrete_events.size();
    }
    
    std::cout << "Monitoring ended, verdict is: " << monitor.status() << "\nMonitored " << event_counter << " events\n";
    */

    /*
    if (vm.count("monitor-interval") || vm.count("monitor-concrete")) {
        std::vector<std::string> input;

        if (vm.count("monitor-interval"))
            input = vm["monitor-interval"].as<std::vector<std::string>>();
        else
            input = vm["monitor-concrete"].as<std::vector<std::string>>();

        if (input.size() != 3) {
            std::cout << "3 arguments required but " << input.size() << " was given" << std::endl;
            print_help(options);
        }

        TA pos = Parser::parse(input[0].c_str(), input[1].c_str());
        TA neg = Parser::parse(input[1].c_str(), input[2].c_str());

        if (vm.count("monitor-interval"))
            do_monitoring<true>(pos, neg);
        else
            do_monitoring<false>(pos, neg);
    }
    */

    return 0;
}