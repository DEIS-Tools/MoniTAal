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

namespace monitaal {

    /**
     * A timed character in a timed word.
     * Consists of a timing element and a label
     */
    struct timed_input_t {
        const float time;
        const label_t label;

        timed_input_t(float time, label_t label);
    };

    enum monitor_answer_e {INCONCLUSIVE, POSITIVE, NEGATIVE};
    /**
     * Monitors a property p from the two TBA's constructed for p and -p
     */
    class Monitor {
        // Private class. Monitors a single automata one step at a time
        enum single_monitor_answer_e {ACTIVE, OUT};
        class Single_monitor { // Bad naming I KNOW
            const TA _automaton;

            // Where it is still possible to reach an accepting location infinitely often
            const symbolic_state_map_t _accepting_space;

            std::vector<state_t> _current_states;

            single_monitor_answer_e _status;

        public:
            Single_monitor(const TA &automaton);

            single_monitor_answer_e status();

            single_monitor_answer_e input(const timed_input_t& input);
        };

        Single_monitor _monitor_pos, _monitor_neg;

        std::vector<timed_input_t> _word;

        monitor_answer_e _status;

    public:
        Monitor(const TA& pos, const TA& neg);

        monitor_answer_e input(const std::vector<timed_input_t>& input);

        monitor_answer_e input(const timed_input_t& input);

        monitor_answer_e status() const;

    };

}

#endif //TIMON_MONITOR_H
