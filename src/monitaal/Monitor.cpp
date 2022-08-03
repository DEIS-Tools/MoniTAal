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

#include "Monitor.h"

#include <utility>
#include <iostream>
#include <type_traits>
#include <cassert>
#include <cstring>

namespace monitaal {

    template<bool is_interval>
    timed_input_t<is_interval>::timed_input_t(typename std::conditional_t<is_interval, interval_t, concrete_time_t> time,
                                              label_t label) : time(time), label(std::move(label)) {}

    template<bool is_interval>
    Monitor<is_interval>::Single_monitor::Single_monitor(const TA &automaton) :
    _automaton(automaton), _accepting_space(Fixpoint::buchi_accept_fixpoint(automaton)) {
        
        typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>
            init(_automaton.initial_location(), _automaton.number_of_clocks());

        if (init.is_included_in(_accepting_space))
            _status = ACTIVE;
        else
            _status = OUT;


        _current_states = std::vector{init};
    }

    template<bool is_interval> typename Monitor<is_interval>::single_monitor_answer_e
    Monitor<is_interval>::Single_monitor::status() { return _status; }

    template<bool is_interval> typename Monitor<is_interval>::single_monitor_answer_e
    Monitor<is_interval>::Single_monitor::input(const timed_input_t<is_interval>& input) {
        std::vector<typename std::conditional_t<is_interval, symbolic_state_t, concrete_state_t>>
                next_states;

        for (auto& s : _current_states) {
            s.delay(input.time);
            if (s.satisfies(_automaton.locations().at(s.location()).invariant()))
                s.restrict(_automaton.locations().at(s.location()).invariant());
            else
                continue;

            auto state = s;
            for (const auto& edge : _automaton.edges_from(s.location()))
                if (std::strcmp(edge.label().c_str(), input.label.c_str()) == 0) //for all edges with input label

                    // If we can do the transition (then do it) and also satisfies the invariant, then explore this
                    if (state.do_transition(edge) && state.satisfies(_automaton.locations()
                                                          .at(edge.to()).invariant())) {
                        state.restrict(_automaton.locations().at(edge.to()).invariant());

                        // Only add the state if it is included in the possible accept space
                        if (state.is_included_in(_accepting_space))
                            next_states.push_back(state);
                        state = s;
                    }
        }

        // Only possible accept states are added. If empty, then we are out
        if (next_states.size() == 0)
            _status = OUT;
        else
            _status = ACTIVE;

        _current_states = next_states;

        return _status;
    }

    template<bool is_interval>
    Monitor<is_interval>::Monitor(const TA& pos, const TA& neg)
            : _monitor_pos(Single_monitor(pos)), _monitor_neg(Single_monitor(neg)) {

        assert((_monitor_pos.status() != OUT || _monitor_neg.status() != OUT) &&
               "Error: Mismatch between positive and negative automata. Both are out\n");
        if (_monitor_pos.status() == OUT)
            _status = NEGATIVE;
        else if (_monitor_neg.status() == OUT)
            _status = POSITIVE;
        else
            _status = INCONCLUSIVE;

    }

    template<bool is_interval>
    monitor_answer_e Monitor<is_interval>::input(const std::vector<timed_input_t<is_interval>> &input) {
        for (const auto& i : input) {
            _status = this->input(i);
            if (_status != INCONCLUSIVE)
                break;
        }

        return _status;
    }

    template<bool is_interval>
    monitor_answer_e Monitor<is_interval>::input(const timed_input_t<is_interval>& input) {
        auto pos = _monitor_pos.input(input), neg = _monitor_neg.input(input);

        assert((pos != OUT || neg != OUT) &&
               "Error: Mismatch between positive and negative automata. Both are out\n");

        if (pos == OUT)
            _status = NEGATIVE;
        if (neg == OUT)
            _status = POSITIVE;

        return _status;
    }

    template<bool is_interval>
    monitor_answer_e Monitor<is_interval>::status() const {
        return _status;
    }

    std::ostream& operator<<(std::ostream &out, const monitor_answer_e value) {
        switch (value) {
            case INCONCLUSIVE: out << "INCONCLUSIVE"; break;
            case POSITIVE: out << "POSITIVE"; break;
            case NEGATIVE: out << "NEGATIVE"; break;
        }
        return out;
    }

    template struct timed_input_t<true>;
    template struct timed_input_t<false>;
    template class Monitor<true>;
    template class Monitor<false>;
}
