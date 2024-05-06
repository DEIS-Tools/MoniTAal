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

    timed_input_t::timed_input_t(interval_t time, label_t label) : time(time), label(std::move(label)) {}
    timed_input_t::timed_input_t(symb_time_t time, label_t label) : time({time, time}), label(std::move(label)) {}

    template<>
    Monitor<delay_state_t>::Single_monitor::Single_monitor(const TA &automaton, const settings_t& setting) :
    _automaton(automaton), 
    _accepting_space(Fixpoint<delay_state_t>::buchi_accept_fixpoint(automaton)),
    _inclusion(setting.inclusion) {
        
        delay_state_t init = delay_state_t(_automaton.initial_location(), _automaton.number_of_clocks(), setting.latency, setting.jitter);

        init.intersection(_accepting_space);
        if (init.is_empty())
            _status = OUT;
        else {
            _status = ACTIVE;
            _current_states = std::vector{init};
        }
    }

    template<class state_t>
    Monitor<state_t>::Single_monitor::Single_monitor(const TA &automaton, const settings_t& setting) :
    _automaton(automaton), 
    _accepting_space(Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(automaton)),
    _inclusion(setting.inclusion) {
        
        state_t init = state_t(_automaton.initial_location(), _automaton.number_of_clocks());

        init.intersection(_accepting_space);
        if (init.is_empty())
            _status = OUT;
        else {
            _status = ACTIVE;
            _current_states = std::vector{init};
        }
    }

    template<class state_t> Monitor<state_t>::single_monitor_answer_e
    Monitor<state_t>::Single_monitor::status() { return _status; }

    template<class state_t> Monitor<state_t>::single_monitor_answer_e
    Monitor<state_t>::Single_monitor::input(const timed_input_t& input) {

        std::vector<state_t> next_states;

        if (input.label == "" || not _automaton.labels().contains(input.label)) { // If label is empty, we do not take any transitions, only delay
            for (auto& s : _current_states) {
                s.delay(input.time);
                if (s.satisfies(_automaton.locations().at(s.location()).invariant())) {
                    s.restrict(_automaton.locations().at(s.location()).invariant());
                    s.intersection(_accepting_space);
                    if (!s.is_empty()) {
                        bool add = true;
                        if (_inclusion) {
                            for (const auto& next_s : next_states) {
                                if (s.is_included_in(next_s))
                                    add = false;
                            }
                        }
                        if (add)
                            next_states.push_back(s);
                    }
                }
            }
        } else {
            for (auto& s : _current_states) {
                s.delay(input.time);
                if (s.satisfies(_automaton.locations().at(s.location()).invariant()))
                    s.restrict(_automaton.locations().at(s.location()).invariant());
                else
                    continue;

                auto state = s;
                for (const auto& edge : _automaton.edges_from(s.location())) {

                    if (std::strcmp(edge.label().c_str(), input.label.c_str()) == 0) { //for all edges with input label

                        // If we can do the transition (then do it) and also satisfies the invariant, then explore this
                        if (state.do_transition(edge) && state.satisfies(_automaton.locations()
                                                            .at(edge.to()).invariant())) {
                            state.restrict(_automaton.locations().at(edge.to()).invariant());

                            // Only add the state if it is included in the possible accept space
                            state.intersection(_accepting_space);
                            if (!state.is_empty()) {
                                bool add = true;
                                if (_inclusion) {
                                    for (const auto& next_s : next_states) {
                                        if (state.is_included_in(next_s))
                                            add = false;
                                    }
                                }
                                if (add)
                                    next_states.push_back(state);
                            }
                        }
                        state = s;
                    }
                }
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

    template<class state_t> std::vector<state_t>
    Monitor<state_t>::Single_monitor::state_estimate() { return _current_states; };

    template<class state_t>
    Monitor<state_t>::Monitor(const TA& pos, const TA& neg)
            : _monitor_pos(Single_monitor(pos, settings_t())), _monitor_neg(Single_monitor(neg, settings_t())) {

        assert((_monitor_pos.status() != OUT || _monitor_neg.status() != OUT) &&
               "Error: Mismatch between positive and negative automata. Both are out\n");
        if (_monitor_pos.status() == OUT)
            _status = NEGATIVE;
        else if (_monitor_neg.status() == OUT)
            _status = POSITIVE;
        else
            _status = INCONCLUSIVE;
    }

    template<class state_t>
    Monitor<state_t>::Monitor(const TA& pos, const TA& neg, const settings_t& setting)
            : _monitor_pos(Single_monitor(pos, setting)), _monitor_neg(Single_monitor(neg, setting)) {

        assert((_monitor_pos.status() != OUT || _monitor_neg.status() != OUT) &&
               "Error: Mismatch between positive and negative automata. Both are out\n");
        if (_monitor_pos.status() == OUT)
            _status = NEGATIVE;
        else if (_monitor_neg.status() == OUT)
            _status = POSITIVE;
        else
            _status = INCONCLUSIVE;

    }

    template<class state_t>
    monitor_answer_e Monitor<state_t>::input(const std::vector<timed_input_t>& input) {
        for (const auto& i : input) {
            _status = this->input(i);
            if (_status != INCONCLUSIVE)
                break;
        }

        return _status;
    }

    template<class state_t>
    monitor_answer_e Monitor<state_t>::input(const timed_input_t& input) {
        auto pos = _monitor_pos.input(input), neg = _monitor_neg.input(input);

        if (pos == OUT && neg == OUT)
            assert((pos != OUT || neg != OUT) &&
                "Error: Mismatch between positive and negative automata. Both are out\n");

        if (pos == OUT)
            _status = NEGATIVE;
        if (neg == OUT)
            _status = POSITIVE;

        return _status;
    }

    template<class state_t>
    monitor_answer_e Monitor<state_t>::status() const {
        return _status;
    }

    template<class state_t>
    std::vector<state_t>
    Monitor<state_t>::positive_state_estimate() {
        return _monitor_pos.state_estimate();
    }

    template<class state_t>
    std::vector<state_t>
    Monitor<state_t>::negative_state_estimate() {
        return _monitor_neg.state_estimate();
    }

    template<class state_t>
    void Monitor<state_t>::Single_monitor::print_status(std::ostream& out) const {
        out << "Number of states: " << _current_states.size() << '\n';
    }

    template<>
    void Monitor<delay_state_t>::Single_monitor::print_status(std::ostream& out) const {
        out << "Consistent latencies: ";
        symb_time_t jitter = 0;

        auto latencies = boost::icl::interval_set<symb_time_t>();

        for (const auto& s : _current_states) {
            latencies += s.get_latency();
            jitter = s.get_jitter_bound();
        }

        out << latencies << "\nJitter bound: " << jitter << '\n';
    }

    template<class state_t>
    void Monitor<state_t>::print_status(std::ostream& out) const {
        out << "Verdict: " << status() << '\n';
        if (status() == INCONCLUSIVE || true) {
            out << "Positive:\n";
            _monitor_pos.print_status(out);
            out << "\nNegative:\n";
            _monitor_neg.print_status(out);
            out << '\n';
        }
    }

    std::ostream& operator<<(std::ostream &out, const monitor_answer_e value) {
        switch (value) {
            case INCONCLUSIVE: out << "INCONCLUSIVE"; break;
            case POSITIVE: out << "POSITIVE"; break;
            case NEGATIVE: out << "NEGATIVE"; break;
        }
        return out;
    }

    template class Monitor<symbolic_state_t>;
    template class Monitor<delay_state_t>;
    template class Monitor<concrete_state_t>;
}
