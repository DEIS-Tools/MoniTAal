#include "fixpoint/Parser.h"
#include "fixpoint/Fixpoint.h"
#include "fixpoint/state_t.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;
using namespace fixpoint;

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

    void print_simple(const TA& T, const states_map_t& states) const {
        std::cout << T << '\n';
        states.print(std::cout, T);
    }

    void write_simple(const TA& T, const states_map_t& states) const {
        std::ofstream out = open_file();
        out << T << '\n';
        states.print(out, T);
        out.close();
    }

public:
    explicit Output(const std::string& caption = "Output Option") : output_options(caption) {
        output_options.add_options()
                ("print,p", "Prints the conclusion (in simple format)")
                ("out,o", po::value<std::string>(&output_file)->implicit_value("out.txt"),
                        "Writes the conclusion to a file in simple format (out.txt by default)");
    }

    [[nodiscard]] const po::options_description& options() const { return output_options; }

    void do_output(po::variables_map vm, const TA& T, const states_map_t& states) const {
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
            ("help,h", "Dispay this help message")
            ("input-model_a", po::value<std::string>()->required(), "Input model for the property");
//            ("input-model_not_a", po::value<std::string>()->required(),
//                    "Input model for the negative property");

    Output output("Output Option");

    options.add(output.options());

    po::positional_options_description model;
    model.add("input-model_a", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).positional(model).run(), vm);

    if (vm.count("help")) {
        std::cout << options << std::endl;
        return 1;
    }

    try {
        po::notify(vm);
    } catch (boost::wrapexcept<po::required_option>& e) {
        std::cerr << "Error: no input model provided\n";
        exit(-1);
    }

    TA T = Parser::parse(vm["input-model_a"].as<std::string>().c_str());

    auto buchi_accept_states = Fixpoint::buchi_accept_fixpoint(T);

    output.do_output(vm, T, buchi_accept_states);

    return 0;
}