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
using conc_ti = concrete_input;

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

    void print_simple(const TA& T, const symbolic_state_map_t& states) const {
        std::cout << T << '\n';
        states.print(std::cout, T);
    }

    void write_simple(const TA& T, const symbolic_state_map_t& states) const {
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
            ("pos", po::value<std::string>()->required(), "Input name of the model for the property")
            ("neg", po::value<std::string>()->required(), "Input name of the model for the negative property")
            ("model,m", po::value<std::string>()->required(), "Path to the input model xml file");

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
        std::cerr << "Error: two input models required\n";
        exit(-1);
    }

    TA pos = Parser::parse(vm["model"].as<std::string>().c_str(), vm["pos"].as<std::string>().c_str());
    TA neg = Parser::parse(vm["model"].as<std::string>().c_str(), vm["neg"].as<std::string>().c_str());

//    output.do_output(vm, T, buchi_accept_states);
    std::vector<conc_ti> word1 = {
            conc_ti(0, "c"),
            conc_ti(2, "b"),
            conc_ti(100, "b"),
            conc_ti(2, "a"),
            conc_ti(0, "c"),
            conc_ti(5, "a"),
            conc_ti(10, "b"),
            conc_ti(0, "c"),
            conc_ti(0, "c")};

    std::vector<conc_ti> word2 = {
            conc_ti(0, "a"),
            conc_ti(101, "c")};


    std::cout << pos << "\n" << neg << "\n";


    Fixpoint::buchi_accept_fixpoint(pos).print(std::cout, pos);
    Fixpoint::buchi_accept_fixpoint(neg).print(std::cout, neg);

    Concrete_monitor monitor(pos, neg);

    auto answer = monitor.input(word1);
    switch (answer) {
        case INCONCLUSIVE: std::cout << "INCONCLUSIVE\n";
            break;
        case POSITIVE: std::cout << "POSITIVE\n";
            break;
        case NEGATIVE: std::cout << "NEGATIVE\n";
            break;
    }

    answer = monitor.input(word2);
    switch (answer) {
        case INCONCLUSIVE: std::cout << "INCONCLUSIVE\n";
            break;
        case POSITIVE: std::cout << "POSITIVE\n";
            break;
        case NEGATIVE: std::cout << "NEGATIVE\n";
            break;
    }

    return 0;
}