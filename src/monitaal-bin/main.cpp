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

    [[nodiscard]] std::ofstream open_file() const {
        std::ofstream out(output_file);
        if (!out.is_open()) {
            std::cerr << "Could not open " << output_file << " for writing\n";
            exit(-1);
        }
        return out;
    }

public:
    static void print_simple(const TA& T, const symbolic_state_map_t& states) {
        std::cout << T << '\n';
        states.print(std::cout, T);
    }

    void write_simple(const TA& T, const symbolic_state_map_t& states) const {
        std::ofstream out = open_file();
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

int main(int argc, const char** argv) {

    po::options_description options;
    options.add_options()
            ("help,h", "Dispay this help message\nExample: monitaal-bin --pos <name> <path> --neg <name> <path>")
            ("pos", po::value<std::string>(), "Input name of the model for the property")
            ("neg", po::value<std::string>(), "Input name of the model for the negative property")
            ("model,m", po::value<std::string>(), "Path to the input model xml file");

    Output output("Output Option");

    options.add(output.options());

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

    if (vm.count("print")) {
        TA T = Parser::parse(vm["print"].as<std::string>().c_str(), "");
        Output::print_simple(T, Fixpoint::buchi_accept_fixpoint(T));
    }

    return 0;
}