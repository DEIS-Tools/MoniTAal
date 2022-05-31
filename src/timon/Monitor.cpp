/*
 * Copyright Thomas M. Grosen 
 * Created on 30/05/2022
 */

/*
 * This file is part of timon
 *
 * timon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * timon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with timon. If not, see <https://www.gnu.org/licenses/>.
 */
#include "Monitor.h"

#include <utility>

namespace timon {

    timed_input_t::timed_input_t(float time, label_t label) : time(time), label(std::move(label)) {}


    Single_monitor::Single_monitor(const TA &automaton) :
    _automaton(automaton), _accepting_space(Fixpoint::buchi_accept_fixpoint(automaton)) {
        symbolic_state_t init(_automaton.initial_location(),
                              Federation::unconstrained(_automaton.number_of_clocks));

        if (init.is_included_in(_accepting_space.at(_automaton.initial_location())))
            _status = ACTIVE;
        else
            _status = OUT;

        _current_states = std::vector<state_t>{state_t(_automaton.initial_location(), _automaton.number_of_clocks)};
    }

    Single_monitor::single_monitor_answer_e Single_monitor::get_status() { return _status; }

    Single_monitor::single_monitor_answer_e Single_monitor::input(timed_input_t input) {
        for (auto& v : _current_states) {
            v.delay(input.time);
            auto tmp = v;
            bool add = false;
            /* for all edges with label input._label {
             *     if (not add && v.transition(edge)) // relies on short circuiting
             *       add = true;
             *     else
             *       auto tmp2 = tmp
             *       if (tmp2.transition(edge))
             *         _current_states.insert(tmp2);
             *  }
             *  if (not add)
             *    remove v from _current_states
             */

        }

        return _status;
    }
}