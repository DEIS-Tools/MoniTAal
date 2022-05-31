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
#ifndef TIMON_MONITOR_H
#define TIMON_MONITOR_H

#include "types.h"
#include "TA.h"
#include "state.h"
#include "Fixpoint.h"

#include <vector>

namespace timon {

    /**
     * A timed character in a timed word.
     * Consists of a timing element and a label
     */
    struct timed_input_t {
        const float time;
        const label_t label;

        timed_input_t(float time, label_t label);
    };

    class Single_monitor {
        enum single_monitor_answer_e {ACTIVE, OUT};

        const TA _automaton;

        // Where it is still possible to reach an accepting location infinitely often
        const symbolic_state_map_t _accepting_space;

        std::vector<timed_input_t> _timed_word;

        std::vector<state_t> _current_states;

        single_monitor_answer_e _status;

    public:
        Single_monitor(const TA &automaton);

        single_monitor_answer_e get_status();

        single_monitor_answer_e input(timed_input_t input);
    };

}

#endif //TIMON_MONITOR_H
