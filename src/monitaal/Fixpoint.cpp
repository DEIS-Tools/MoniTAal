/*
 * Copyright Thomas M. Grosen 
 * Created on 23/05/2022
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

#include "Fixpoint.h"
#include "types.h"
#include "state.h"

namespace monitaal {

    template<class state_t>
    state_map_t<state_t> Fixpoint<state_t>::reach(const state_map_t<state_t>& states, const TA& T) {
        state_map_t<state_t> waiting;
        state_map_t<state_t> passed;

        // We have to take at least one step
        for (const auto& [_, s] : states) {
            for (const auto& e : T.edges_to(s.location())) {
                auto state = s;
                state.do_transition_backward(e);
                state.restrict(T.locations().at(e.from()).invariant());
                waiting.insert(state); //Checks for emptyness of the state before inserting
            }
        }

        while (not waiting.is_empty()) {
            state_t s = waiting.begin()->second;
            waiting.remove(s.location());

            if (passed.has_state(s.location()) && s.is_included_in(passed.at(s.location())))
                continue;

            passed.insert(s);

            for (const auto& e : T.edges_to(s.location())) {
                auto pred = s;
                pred.do_transition_backward(e);
                pred.restrict(T.locations().at(e.from()).invariant());
                waiting.insert(pred);
            }
        }

        return passed;
    }

    template<class state_t>
    state_map_t<state_t> Fixpoint<state_t>::accept_states(const TA &T) {
        state_map_t<state_t> accept_states;

        for (const auto& [_, loc] : T.locations()) {
            if (loc.is_accept())
                accept_states.insert(state_t::unconstrained(loc.id(), T.number_of_clocks()));
        }

        return accept_states;
    }

    template<class state_t>
    state_map_t<state_t> Fixpoint<state_t>::buchi_accept_fixpoint(const TA &T) {
        auto reach_a = reach(accept_states(T), T);


        std::vector<location_id_t> erase_list{};
        // Remove states in all locations that are not accept. This is the same as intersecting with accept states
        for (const auto &[l,_] : reach_a)
            if (not T.locations().at(l).is_accept())
                erase_list.push_back(l);

        for (const auto &l : erase_list)
            reach_a.remove(l);
        erase_list.clear();

        auto reach_b = reach(reach_a, T);

        while (not reach_a.equals(reach_b)) {
            reach_a = reach_b;

            for (const auto &[l,_] : reach_b)
                if (not T.locations().at(l).is_accept())
                    erase_list.push_back(l);

            for (const auto &l : erase_list)
                reach_b.remove(l);
            erase_list.clear();

            reach_b = reach(reach_b, T);
        }

        return reach_a;
    }

    template class Fixpoint<symbolic_state_t>;
    template class Fixpoint<delay_state_t>;

}