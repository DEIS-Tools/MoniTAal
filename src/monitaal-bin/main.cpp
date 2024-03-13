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

namespace po = boost::program_options;
using namespace monitaal;

class Output {
private:
    po::options_description output_options;
    po::variables_map vm;
    std::string output_file;

public:
    static [[nodiscard]] std::ofstream open_file(const std::string& file) {
        std::ofstream out(file);
        if (!out.is_open()) {
            std::cerr << "Could not open " << file << " for writing\n";
            exit(-1);
        }
        return out;
    }

    static void print_simple(const TA& T, const symbolic_state_map_t& states) {
        std::cout << T << '\n';
        states.print(std::cout, T);
    }

    void write_simple(const TA& T, const symbolic_state_map_t& states) const {
        std::ofstream out = open_file(output_file);
        out << T << '\n';
        states.print(out, T);
        out.close();
    }

    explicit Output(const std::string& caption = "Output Option") : output_options(caption) {
        output_options.add_options()
                ("print,p", "Print the analysis in simple format")
                ("out,o", po::value<std::string>(&output_file)->implicit_value("out.txt"),
                        "Writes the output to a file (out.txt by default)");
    }

    [[nodiscard]] const po::options_description& options() const { return output_options; }

    void do_output(po::variables_map vm, const TA& T, const symbolic_state_map_t& states) const {
        if (vm.count("print")) {
            if (vm.count("out")) {
                //output_file = vm["out"].as<std::string>();
                write_simple(T, states);
            }

            print_simple(T, states);
        }
    }
};

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

struct settings_t
{
    bool verbose = false;
    uint32_t event_counter = 0;
    TA positive, negative;

    settings_t(const TA& positive, const TA& negative) : positive(positive), negative(negative) {};
};

template<bool is_interval>
void interactive_monitoring(Monitor<is_interval> monitor, settings_t settings, std::ostream& out, std::istream& in) {
    std::string input = "";

    std::vector<typename std::conditional_t<is_interval, interval_input, concrete_input>>
    events;

    if (monitor.status() != INCONCLUSIVE)
        out << "Interactive monitor (respond with \"q\" to quit)\n";

    while (monitor.status() == INCONCLUSIVE) {
        if (settings.verbose) {
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
                events = EventParser::parse_input<is_interval>(&in);
            }
            catch(const base_error& e)
            {
                std::cout << e.what() << '\n';
                continue;
            }
            break;
        }
        if (input == "q") break;

        monitor.input(events);
        settings.event_counter += events.size();
    }
    
    std::cout << "Monitoring ended, verdict is: " << monitor.status() << "\nMonitored " << settings.event_counter << " events\n";
}

template void interactive_monitoring<true>(Monitor<true> monitor, settings_t settings, std::ostream& out, std::istream& in);
template void interactive_monitoring<false>(Monitor<false> monitor, settings_t settings, std::ostream& out, std::istream& in);



int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-bin --pos <name> <path> --neg <name> <path>")
            ("pos,p", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> Property automaton.")
            ("neg,n", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file> Negated property automaton.")
            ("type, t", po::value<std::string>(), "Monitoring procedure (concrete or interval) default = concrete")
            ("input,i", po::value<std::string>(), "Monitor events contained in file.")
            ("verbose,v", "Prints more information on the monitoring procedure")
            ("print-dot,o", "Prints the dot graphs of the given automata")
            ("div,d", po::value<std::vector<std::string>>()->multitoken(), "<list of labels> Take time divergence into account.");

    //Output output("Output Option");

    //options.add(output.options());

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

    bool is_interval = false;
    if (vm.count("type")) {
        std::string type = vm["type"].as<std::string>();
        if (type == "interval")
            is_interval = true;
        else if (type == "concrete")
            is_interval = false;
        else {
            std::cerr << "Error: type must be concrete or interval\n";
            exit(-1);
        }
    }

    if (vm.count("print-dot")) {
        std::cout << "DOT GRAPHS\n";
        pos.print_dot(std::cout);
        std::cout << '\n';
        neg.print_dot(std::cout);
        std::cout << '\n';
    }

    if (vm.count("div")) {
        auto alphabet = vm["div"].as<std::vector<std::string>>();
        TA div = TA::time_divergence_ta(alphabet, true);
        pos.intersection(div);
        neg.intersection(div);

        if (vm.count("print-dot")) {
            std::cout << "DOT GRAPHS with time divergence conjunction\n";
            pos.print_dot(std::cout);
            std::cout << '\n';
            neg.print_dot(std::cout);
            std::cout << '\n';
        }

    }

    settings_t settings(pos, neg);
    settings.verbose = vm.count("verbose") > 0;

    Interval_monitor monitor_int(pos, neg);
    Concrete_monitor monitor_con(pos, neg);

    // Monitoring events from file

    /*
    if (vm.count("input")) {
        auto inputarg = vm["input"].as<std::string>();
        std::ifstream input;
        input.open(inputarg);

        char read[256];

        while (not input.eof() || monitor.status() == INCONCLUSIVE) {    
            input.getline(read, 256);
            
            if (input.gcount() == 0) // If the line is empty
                continue;

            assert(not input.fail() && "Error: Too many characters in one line");
    
            membuf sbuf(read, read + input.gcount());
            auto in = std::istream(&sbuf);

            if (concrete) {
                concrete_events = EventParser::parse_concrete_input(&in);
                monitor.input(concrete_events);
                event_counter += concrete_events.size();
            }
            else {
                interval_events = EventParser::parse_interval_input(&in);
                interval_monitor.input(interval_events);
                event_counter += interval_events.size();
            }

        }

        input.close();
        std::cout << "Monitored " << inputarg << "\n";
    }*/


    // Interactive Monitoring

    if (is_interval)
        interactive_monitoring<true>(monitor_int, settings, std::cout, std::cin);
    else
        interactive_monitoring<false>(monitor_con, settings, std::cout, std::cin);
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