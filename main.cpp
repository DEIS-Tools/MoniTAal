#include "Parser.h"
#include "Fixpoint.h"
#include "state_t.h"

#include <iostream>
using namespace fixpoint;

int main() {
    TA T = Parser::parse("../tree.xml");

    std::cout << T << '\n' << T.number_of_clocks;

    states_map_t accept_states = Fixpoint::accept_states(T);

    states_map_t reach_1 = Fixpoint::reach(accept_states, T);

    std::cout << "intersecting\n";
    reach_1.intersection(accept_states);
    std::cout << "intersected\n";

    states_map_t reach_2 = Fixpoint::reach(reach_1, T);

    reach_2.print(std::cout, T);

    return 0;
}
