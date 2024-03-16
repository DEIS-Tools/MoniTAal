/*
 * Copyright Thomas M. Grosen 
 * Created on 30/05/2022
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

#ifndef MONITAAL_MONITOR_H
#define MONITAAL_MONITOR_H

#include "types.h"
#include "TA.h"
#include "state.h"
#include "Fixpoint.h"

#include <vector>
#include <type_traits>

namespace monitaal {

    /**
     * A timed character in a timed word.
     * Consists of a timing element and a label
     */
    template<bool is_interval>
    struct timed_input_t {
        typename std::conditional_t<is_interval, interval_t, concrete_time_t>
                 const time;
        const label_t label;

        timed_input_t(typename std::conditional_t<is_interval, interval_t, concrete_time_t> time, label_t label);
    };

    enum monitor_answer_e {INCONCLUSIVE, POSITIVE, NEGATIVE};

    std::ostream& operator<<(std::ostream& out, const monitor_answer_e value);

    /**
     * Monitors a property p from the two TBA's constructed for p and -p
     */
    template<bool is_interval>
    class Monitor {

        enum single_monitor_answer_e {ACTIVE, OUT};

        // Private class. Monitors a single automata one step at a time
        class Single_monitor { // Bad naming I KNOW
            const TA _automaton;

            // Where it is still possible to reach an accepting location infinitely often
            const symbolic_state_map_t _accepting_space;

            std::vector<typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>>
                _current_states;

            single_monitor_answer_e _status;

            bool inclusion;

        public:
            explicit Single_monitor(const TA &automaton, bool inclusion);

            single_monitor_answer_e status();

            single_monitor_answer_e input(const timed_input_t<is_interval>& input);

            std::vector<typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>>
            state_estimate();
        };

        Single_monitor _monitor_pos, _monitor_neg;

        std::vector<timed_input_t<is_interval>> _word;

        monitor_answer_e _status;

    public:
        Monitor(const TA& pos, const TA& neg, bool inclusion);
        Monitor(const TA& pos, const TA& neg);

        monitor_answer_e input(const std::vector<timed_input_t<is_interval>>& input);

        monitor_answer_e input(const timed_input_t<is_interval>& input);

        std::vector<typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>>
        positive_state_estimate();

        std::vector<typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>>
        negative_state_estimate();

        [[nodiscard]] monitor_answer_e status() const;

    };

}

#endif //MONITAAL_MONITOR_H
