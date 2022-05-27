/*
 * Copyright Thomas M. Grosen 
 * Created on 23/05/2022
 */

/*
 * This file is part of fixpoint
 *
 * fixpoint is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fixpoint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with fixpoint. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Fixpoint.h"
#include "types.h"
#include "state_t.h"

namespace fixpoint {

    states_map_t Fixpoint::reach(const states_map_t &states, const TA& T) {
        states_map_t waiting;
        states_map_t passed;

//         We have to take at least one step
        for (const auto& [_, s] : states) {
            for (const auto& e : T.edges_to(s.location_id())) {
                auto state = s;
                state.step_back(e);
                waiting.insert(state);
            }
        }

        while (not waiting.is_empty()) {
            auto s = waiting.begin()->second;
            waiting.remove(s.location_id());

            if (passed.has_state(s.location_id()) && s.is_included_in(passed.at(s.location_id())))
                continue;

            passed.insert(s);

            for (const auto& e : T.edges_to(s.location_id())) {
                 auto pred = s;
                 pred.step_back(e);
                 waiting.insert(pred);
            }
        }

        return passed;
    }

    states_map_t Fixpoint::accept_states(const TA &T) {
        states_map_t accept_states;

        for (const auto& [_, loc] : T.locations()) {
            if (loc.is_accept())
                accept_states.insert(state_t(loc.id(), Federation::unconstrained(T.number_of_clocks)));
        }

        return accept_states;
    }

    states_map_t Fixpoint::buchi_accept_fixpoint(const TA &T) {
        auto acc = accept_states(T);
        auto intersection = acc;
        auto reach_a = reach(acc, T);
        intersection.intersection(reach_a);
        auto reach_b = reach(intersection, T);

        while (not reach_a.equals(reach_b)) {
            reach_a = reach_b;
            intersection = acc;

            intersection.intersection(reach_b);

            reach_b = reach(intersection, T);
        }

        return reach_a;
    }

}