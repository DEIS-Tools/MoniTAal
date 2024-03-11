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

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace po = boost::program_options;
using namespace monitaal;

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

void run_benchmark(TA& pos, TA& neg, std::istream& input) {
    char read[256];
    
    Concrete_monitor monitor(pos, neg);

    int event_counter = 0;

    auto t1 = std::chrono::high_resolution_clock::now();

    while (not input.eof() || monitor.status() == INCONCLUSIVE) {    
        input.getline(read, 256);
        
        if (input.gcount() == 0) // If the line is empty
            continue;

        assert(not input.fail() && "Error: Too many characters in one line");

        membuf sbuf(read, read + input.gcount());
        auto in = std::istream(&sbuf);

        auto events = EventParser::parse_concrete_input(&in);

        monitor.input(events);
        ++event_counter;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

    std::cout << "Monitored " << event_counter << " events in " << time.count() << "ms\nMonitor verdict is " << monitor.status() << '\n';
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-benchmark --pos <name> <path> --neg <name> <path> --input <path>")
            ("pos,p", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file>")
            ("neg,n", po::value<std::vector<std::string>>()->required()->multitoken(), "<name of template> <path to xml file>")
            ("input,i", po::value<std::string>()->required(), "Path to file containing observations")
            ("div,d", po::value<std::vector<std::string>>(), "<list of labels> Take time divergence into account.");

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
    auto inputarg = vm["input"].as<std::string>();

    if (posarg.size() != 2 || negarg.size() != 2) {
        for (int i = 0; i < posarg.size(); ++i)
            std::cout << posarg[i];
        std::cerr << "Error: --pos and --neg args require two arguments\n";
        exit(-1);
    }

    TA pos = Parser::parse(&posarg[1][0], &posarg[0][0]);
    TA neg = Parser::parse(&negarg[1][0], &negarg[0][0]);

    if (vm.count("div")) {
        auto alphabet = vm["div"].as<std::vector<std::string>>();
        TA div = TA::time_divergence_ta(alphabet, true);
        pos.intersection(div);
        neg.intersection(div);
    }

    std::ifstream input;
    input.open(inputarg);

    run_benchmark(pos, neg, input);

    input.close();

    return 0;
}