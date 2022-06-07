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

namespace monitaal {

    timed_input_t::timed_input_t(float time, label_t label) : time(time), label(std::move(label)) {}

    template<class STATE>
    Monitor<STATE>::Single_monitor::Single_monitor(const TA &automaton) :
    _automaton(automaton), _accepting_space(Fixpoint::buchi_accept_fixpoint(automaton)) {

        STATE init(_automaton.initial_location(), _automaton.number_of_clocks);

        if (init.is_included_in(_accepting_space))
            _status = ACTIVE;
        else
            _status = OUT;

        _current_states = std::vector<STATE>{init};
    }

    template<class STATE> typename Monitor<STATE>::single_monitor_answer_e
    Monitor<STATE>::Single_monitor::status() { return _status; }

    template<class STATE> typename Monitor<STATE>::single_monitor_answer_e
    Monitor<STATE>::Single_monitor::input(const timed_input_t& input) {
        std::vector<STATE> next_states;

        for (auto& s : _current_states) {
            s.delay(input.time);
            auto state = s;
            for (const auto& edge : _automaton.edges_from(s.location()))
                if (std::strcmp(edge.label().c_str(), input.label.c_str()) == 0) //for all edges with input label
                    if (state.do_transition(edge)) {
                        next_states.push_back(state);
                        state = s;
                    }
        }
        _current_states = next_states;
        // If one of the states are inside, we are still active
        // TODO: remove states that fall outside
        for (const auto& state : _current_states)
            if (state.is_included_in(_accepting_space)) {
                _status = ACTIVE;
                return _status;
            }

        _status = OUT;
        return _status;
    }

    template<class STATE>
    Monitor<STATE>::Monitor(const TA& pos, const TA& neg)
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

    template<class STATE>
    monitor_answer_e Monitor<STATE>::input(const std::vector<timed_input_t> &input) {
        for (const auto& i : input) {
            _status = this->input(i);
            if (_status != INCONCLUSIVE)
                break;
        }

        return _status;
    }

    template<class STATE>
    monitor_answer_e Monitor<STATE>::input(const timed_input_t &input) {
        auto pos = _monitor_pos.input(input), neg = _monitor_neg.input(input);

        assert((pos != OUT || neg != OUT) &&
               "Error: Mismatch between positive and negative automata. Both are out\n");

        if (pos == OUT)
            _status = NEGATIVE;
        if (neg == OUT)
            _status = POSITIVE;

        return _status;
    }

    template<class STATE>
    monitor_answer_e Monitor<STATE>::status() const {
        return _status;
    }

    template class Monitor<symbolic_state_t>;
    template class Monitor<concrete_state_t>;
}
