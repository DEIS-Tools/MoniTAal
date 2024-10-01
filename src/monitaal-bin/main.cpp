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

#include <time.h>
#include <chrono>
#include <stdlib.h>

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
    auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t2);

    for (int i = 0; i < limit-1; ++i) {
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
    monitor.input({11+limit, "b"});
    t2 = std::chrono::high_resolution_clock::now();
    res_tmp = ms_int.count();
    ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << '\n';
    return;
}

void run_benchmark_interval(Interval_monitor& monitor, bin_settings_t& settings, int limit, bool overlap) {
    int max_states = 0, tmp;
    int max_response = 0, res_tmp;
    auto t1 = std::chrono::high_resolution_clock::now(),
         t2 = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t2);

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
        ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
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
    ms_int += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    res_tmp = ms_int.count() - res_tmp;
    max_response = res_tmp > max_response ? res_tmp : max_response;

    tmp = monitor.positive_state_estimate().size() + monitor.negative_state_estimate().size();
    max_states = tmp > max_states ? tmp : max_states;

    std::cout << "\nTime total: " << ms_int.count() << " ns\nMax response time: " << max_response << " ns\nMax states: " << max_states << '\n';
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

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-bin --pos <name> <path> --neg <name> <path>")
            ("pos,p", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> : Property automaton.")
            ("neg,n", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> : Negated property automaton.")
            ("type,t", po::value<std::string>()->default_value("concrete", "concrete"), "Input type (concrete or interval) default = concrete.")
            ("input,i", po::value<std::string>(), "Monitor events contained in file.")
            ("inclusion,u", "Enable inclusion checking for duplicate states")
            ("clock-abstraction,c", "Enable abstraction of inactive clocks (Automatically enables inclusion)")
            ("verbose,v", "Prints more information on the monitoring procedure.")
            ("silent,s", "removes all outputs")
            ("print-dot,o", "Prints the dot graphs of the given automata.")
            ("benchmark", po::value<int>(), "Run predefined benchmark")
            ("advance,a", "For bencharking. Make sure that time points advance")
            ("div,d", po::value<std::vector<std::string>>()->multitoken(), "<list of labels> : Take time divergence into account.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

    if (vm.count("help")) {
        std::cout << options << std::endl;
        return 1;
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

    TA pos = Parser::parse_file(&posarg[1][0], &posarg[0][0]);
    TA neg = Parser::parse_file(&negarg[1][0], &negarg[0][0]);

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
    mon_setting.clock_abstraction = vm.count("clock-abstraction");

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