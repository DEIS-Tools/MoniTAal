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

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>

#include <vector>
#include <type_traits>

namespace monitaal {

    enum input_type_e {ONCE, OPTIONAL, MULTI};
    /**
     * A timed character in a timed word.
     * Consists of a timing element and a label
     */
    struct timed_input_t {
        const interval_t time;
        const label_t label;
        input_type_e type = ONCE;

        timed_input_t(interval_t time, label_t label, input_type_e type = ONCE);
        timed_input_t(symb_time_t time, label_t label, input_type_e type = ONCE);
    };

    struct settings_t {
        bool inclusion = false;
        bool clock_abstraction = false;
        interval_t latency{0,0}, latency_i{0,0};
        symb_time_t jitter = 0, jitter_i = 0;

        settings_t(){};
        settings_t(bool inclusion, bool clock_abstraction, interval_t latency, symb_time_t jitter) : 
                inclusion(inclusion), clock_abstraction(clock_abstraction), latency(latency), jitter(jitter){};
    };

    enum monitor_answer_e {INCONCLUSIVE, POSITIVE, NEGATIVE};

    std::ostream& operator<<(std::ostream& out, const monitor_answer_e value);
    
    enum single_monitor_answer_e {ACTIVE, OUT};
    // Monitors a single automata one step at a time
    template<class state_t>
    class Single_monitor { // Bad naming I KNOW
        const TA _automaton;

        // Where it is still possible to reach an accepting location infinitely often
        const symbolic_state_map_t<typename std::conditional_t<std::is_base_of<symbolic_state_base, state_t>::value, 
        state_t, symbolic_state_t>> _accepting_space;

        std::vector<state_t> _current_states;

        single_monitor_answer_e _status;

        bool _inclusion,
             _clock_abstraction;

    public:
        explicit Single_monitor(const TA &automaton, const settings_t& setting);

        single_monitor_answer_e status();

        single_monitor_answer_e input(const timed_input_t& input);

        std::vector<state_t> state_estimate();

        void print_status(std::ostream& out) const;
    };
    /**
     * Monitors a property p from the two TBA's constructed for p and -p
     */
    template<class state_t>
    class Monitor {

        Single_monitor<state_t> _monitor_pos, _monitor_neg;

        monitor_answer_e _status;

    public:
        Monitor(const TA& pos, const TA& neg, const settings_t& setting);
        Monitor(const TA& pos, const TA& neg);

        monitor_answer_e input(const std::vector<timed_input_t>& input);

        monitor_answer_e input(const timed_input_t& input);

        std::vector<state_t> positive_state_estimate();

        std::vector<state_t> negative_state_estimate();

        [[nodiscard]] monitor_answer_e status() const;

        void print_status(std::ostream& out) const;

    };

}

#endif //MONITAAL_MONITOR_H
