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

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;
using namespace monitaal;

class Output {
private:
    po::options_description output_options;
    po::variables_map vm;
    std::string output_file;

public:
    [[nodiscard]] std::ofstream open_file(const std::string& file) const {
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
        std::ofstream out = open_file("out.txt");
        out << T << '\n';
        states.print(out, T);
        out.close();
    }

    explicit Output(const std::string& caption = "Output Option") : output_options(caption) {
        output_options.add_options()
                ("print,p", po::value<std::string>(), "Print the analysis in simple format")
                ("out,o", po::value<std::string>(&output_file)->implicit_value("out.txt"),
                        "Writes the conclusion to a file in simple format (out.txt by default)");
    }

    [[nodiscard]] const po::options_description& options() const { return output_options; }

    void do_output(po::variables_map vm, const TA& T, const symbolic_state_map_t& states) const {
        if (vm.count("print"))
            print_simple(T, states);
        if (vm.count("out")) {
            write_simple(T, states);
        }
    }
};

void print_help(const po::options_description& options) {
    std::cout << options << std::endl;
}

template <bool is_interval>
void do_monitoring(const TA& pos, const TA& neg) {
    auto monitor = Monitor<is_interval>(pos, neg);

    /*
     * read input from stdin
     * while (monitor.input(input) == INCONCLUSIVE) {
     *    word = readstdin;
     * }
     *
     */
    if (monitor.status() != INCONCLUSIVE) {
        std::cout << "Monitoring ended, status is:\n" << monitor.status() << std::endl;
    }
}

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-bin --pos <name> <path> --neg <name> <path>")
            ("monitor-interval,i", po::value<std::vector<std::string>>(), "<xml file with \u03D5 and \u00AC\u03D5 models> <name of \u03D5> <name of \u00AC\u03D5>");
            ("monitor-concrete,m", po::value<std::vector<std::string>>(), "<xml file with \u03D5 and \u00AC\u03D5 models> <name of \u03D5> <name of \u00AC\u03D5>");

    Output output("Output Option");

    options.add(output.options());

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);

    if (vm.count("help")) {
        print_help(options);
        return 1;
    }

    try {
        po::notify(vm);
    } catch (boost::wrapexcept<po::required_option>& e) {
        std::cerr << "Error: Some required fields not given\n";
        exit(-1);
    }

    if (vm.count("print")) {
        TA T = Parser::parse(vm["print"].as<std::string>().c_str(), "");
        Output::print_simple(T, Fixpoint::buchi_accept_fixpoint(T));
    } else if (vm.count("monitor-interval") || vm.count("monitor-concrete")) {
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

    return 0;
}