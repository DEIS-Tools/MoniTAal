/*
 * Copyright Thomas M. Grosen 
 * Created on 24/05/2022
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

#include "state.h"
#include "types.h"
#include "TA.h"
#include "errors.h"

#include <pardibaal/Federation.h>
#include <iostream>
#include <algorithm>
#include <cassert>

namespace monitaal {

    symbolic_state_t::symbolic_state_t() {
        _location = 0;
        _federation = Federation();
    }

    symbolic_state_t::symbolic_state_t(location_id_t location, clock_index_t clocks) : 
            symbolic_state_base(location, clocks + 1) {}

    symbolic_state_t symbolic_state_t::unconstrained(location_id_t location, clock_index_t clocks) {
        auto rtn = symbolic_state_t();
        rtn._location = location;
        rtn._federation = Federation::unconstrained(clocks + 1);
        return rtn;
    }

    void symbolic_state_t::intersection(const symbolic_state_map_t<symbolic_state_t>& map) {
        if (map.has_state(this->_location))
            intersection(map.at(this->_location));
        else
            this->_federation.restrict(0,0, {-1, true});
    }

    void symbolic_state_t::intersection(const symbolic_state_base& state) {
        symbolic_state_base::intersection(state);
    }

    void symbolic_state_t::delay(symb_time_t value) {
        _federation.future();
        _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_federation.dimension() - 1, value));
        _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_federation.dimension() - 1, value));
    }

    void symbolic_state_t::delay(interval_t interval) {
        _federation.future();
        _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_federation.dimension() - 1, interval.first));
        _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_federation.dimension() - 1, interval.second));
    }

    bool symbolic_state_t::is_included_in(const symbolic_state_map_t<symbolic_state_t>& map) const {
        if (not map.has_state(_location))
            return false;

        return is_included_in(map.at(_location));
    }

    bool symbolic_state_t::is_included_in(const symbolic_state_base &state) const {
        return symbolic_state_base::is_included_in(state);
    }

    delay_state_t::delay_state_t() : _jitter(0) {}

    delay_state_t::delay_state_t(location_id_t location, clock_index_t clocks, interval_t latency, symb_time_t jitter) : 
            symbolic_state_base(location, clocks + 2), _jitter(jitter) {
        _etime = clocks;
        _time = clocks + 1;
        
        _federation.free(_etime);
        _federation.restrict(_time, _etime, pardibaal::bound_t::non_strict(-latency.first));
        _federation.restrict(_etime, _time, pardibaal::bound_t::non_strict(latency.second));
    }

    delay_state_t delay_state_t::unconstrained(location_id_t location, clock_index_t clocks) {
        auto state = delay_state_t();

        state._location = location;
        state._federation = Federation::unconstrained(clocks + 2);
        state._etime = clocks;
        state._time = clocks + 1;

        return state;
    }

    void delay_state_t::intersection(const symbolic_state_base& state) {
        symbolic_state_base::intersection(state);
    }

    void delay_state_t::intersection(const symbolic_state_map_t<delay_state_t>& map) {
        if (map.has_state(this->_location))
            intersection(map.at(this->_location));
        else
            this->_federation.restrict(0,0, {-1, true});
    }

    void delay_state_t::delay(symb_time_t value) {
        _federation.future();
        _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime, value - _jitter));
        _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime, value));
    }

    void delay_state_t::delay(interval_t interval) {
        _federation.future();
        _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime, interval.first - _jitter));
        _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime, interval.second));
    }

    boost::icl::interval_set<symb_time_t> delay_state_t::get_latency() const {
        auto latencies = boost::icl::interval_set<symb_time_t>();

        for (const auto dbm : _federation) {
            auto lower = dbm.at(_time, _etime),
                 upper = dbm.at(_etime, _time);
            if (lower.is_strict()) {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::left_open(-lower.get_bound(), upper.get_bound());
            } else {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::right_open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::closed(-lower.get_bound(), upper.get_bound());
            }
        }

        return latencies;
    }

    symb_time_t delay_state_t::get_jitter_bound() const { return _jitter; }

    bool delay_state_t::is_included_in(const symbolic_state_map_t<delay_state_t>& map) const {
        if (not map.has_state(_location))
            return false;

        return is_included_in(map.at(_location));
    }

    bool delay_state_t::is_included_in(const symbolic_state_base& state) const {
        return symbolic_state_base::is_included_in(state);
    }

    testing_state_t::testing_state_t() : _jitter_o(0), _jitter_i(0) {}

    testing_state_t::testing_state_t(location_id_t location, clock_index_t clocks, interval_t latency_o, interval_t latency_i, symb_time_t jitter_o, symb_time_t jitter_i) : 
            symbolic_state_base(location, clocks + 3), _jitter_o(jitter_o), _jitter_i(jitter_i) {
        _etime_o = clocks;
        _etime_i = clocks + 1;
        _time = clocks + 2;
        
        _federation.free(_etime_o);
        _federation.restrict(_time, _etime_o, pardibaal::bound_t::non_strict(-latency_o.first));
        _federation.restrict(_etime_o, _time, pardibaal::bound_t::non_strict(latency_o.second));

        // Same for input. We don't need to shift, since we expect an input first.
        _federation.future();
        _federation.free(_etime_i);
        _federation.restrict(_time, _etime_i, pardibaal::bound_t::non_strict(latency_i.second));
        _federation.restrict(_etime_i, _time, pardibaal::bound_t::non_strict(-latency_i.first));

        assert(_federation.at(0).at(_time, 0) == pardibaal::bound_t::non_strict(latency_i.second));
        assert(_federation.at(0).at(0, _time) == pardibaal::bound_t::non_strict(-latency_i.first));
    }

    testing_state_t testing_state_t::unconstrained(location_id_t location, clock_index_t clocks) {
        auto state = testing_state_t();

        state._location = location;
        state._federation = Federation::unconstrained(clocks + 3);
        state._etime_o = clocks;
        state._etime_i = clocks + 1;
        state._time = clocks + 2;

        return state;
    }

    void testing_state_t::intersection(const symbolic_state_base& state) {
        symbolic_state_base::intersection(state);
    }

    void testing_state_t::intersection(const symbolic_state_map_t<testing_state_t>& map) {
        if (map.has_state(this->_location))
            intersection(map.at(this->_location));
        else
            this->_federation.restrict(0,0, {-1, true});
    }

    void testing_state_t::delay(symb_time_t value) {
        _federation.future();
        if (_is_input_mode) {
            _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime_i, value - _jitter_i));
            _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime_i, value));
        } else {
            _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime_o, value - _jitter_o));
            _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime_o, value));
        }

        switch_input_mode();
    }

    void testing_state_t::delay(interval_t interval) {
        _federation.future();
        if (_is_input_mode) {
            _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime_i, interval.first - _jitter_i));
            _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime_i, interval.second));
        } else {
            _federation.restrict(pardibaal::difference_bound_t::lower_non_strict(_etime_o, interval.first - _jitter_o));
            _federation.restrict(pardibaal::difference_bound_t::upper_non_strict(_etime_o, interval.second));
        }
        
        switch_input_mode();
    }

    boost::icl::interval_set<symb_time_t> testing_state_t::get_input_latency() const {
        auto latencies = boost::icl::interval_set<symb_time_t>();

        for (const auto dbm : _federation) {
            auto lower = dbm.at(_time, _etime_o),
                 upper = dbm.at(_etime_o, _time);
            if (lower.is_strict()) {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::left_open(-lower.get_bound(), upper.get_bound());
            } else {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::right_open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::closed(-lower.get_bound(), upper.get_bound());
            }
        }

        return latencies;
    }

    boost::icl::interval_set<symb_time_t> testing_state_t::get_output_latency() const {
        auto latencies = boost::icl::interval_set<symb_time_t>();

        for (const auto dbm : _federation) {
            auto upper = dbm.at(_time, _etime_i),
                lower = dbm.at(_etime_i, _time);
            if (lower.is_strict()) {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::left_open(-lower.get_bound(), upper.get_bound());
            } else {
                latencies += upper.is_strict() ? 
                    boost::icl::interval<symb_time_t>::right_open(-lower.get_bound(), upper.get_bound()) :
                    boost::icl::interval<symb_time_t>::closed(-lower.get_bound(), upper.get_bound());
            }
        }

        return latencies;
    }

    symb_time_t testing_state_t::get_input_jitter() const { return _jitter_i; }

    symb_time_t testing_state_t::get_output_jitter() const { return _jitter_o; }

    bool testing_state_t::is_included_in(const symbolic_state_map_t<testing_state_t>& map) const {
        if (not map.has_state(_location))
            return false;

        return is_included_in(map.at(_location));
    }

    bool testing_state_t::is_included_in(const symbolic_state_base& state) const {
        return symbolic_state_base::is_included_in(state);
    }

    template<class state_t>
    void symbolic_state_map_t<state_t>::insert(state_t state) {

        if (not state.is_empty()) {
            if (not this->has_state(state.location())) {
                _states.insert({state.location(), state});
            } else {
                _states[state.location()].add(state);
            }
        }
    }

    template<class state_t>
    void symbolic_state_map_t<state_t>::remove(location_id_t loc) {
        _states.erase(loc);
    }

    template<class state_t>
    state_t symbolic_state_map_t<state_t>::at(location_id_t loc) const {
        return _states.at(loc);
    }

    template<class state_t>
    state_t &symbolic_state_map_t<state_t>::operator[](location_id_t loc) {
        return _states[loc];
    }

    template<class state_t>
    bool symbolic_state_map_t<state_t>::is_empty() const {
        return _states.empty();
    }

    template<class state_t>
    size_t symbolic_state_map_t<state_t>::size() const {
        return _states.size();
    }

    template<class state_t>
    bool symbolic_state_map_t<state_t>::has_state(location_id_t loc) const {
        return _states.find(loc) != _states.end();
    }

    template<class state_t>
    void symbolic_state_map_t<state_t>::intersection(const symbolic_state_map_t<state_t>& states) {
        std::vector<location_id_t> erase_list;

        for(auto &[l, _] : this->_states) {
            if (states.has_state(l)) {
                this->_states[l].intersection(states.at(l));
            } else {
                erase_list.push_back(l);
            }
        }

        for (const auto& l : erase_list)
            remove(l);
    }

    template<class state_t>
    std::map<location_id_t, state_t>::iterator symbolic_state_map_t<state_t>::begin() {
        return _states.begin();
    }

    template<class state_t>
    std::map<location_id_t, state_t>::const_iterator symbolic_state_map_t<state_t>::begin() const {
        return _states.begin();
    }

    template<class state_t>
    std::map<location_id_t, state_t>::iterator symbolic_state_map_t<state_t>::end() {
        return _states.end();
    }

    template<class state_t>
    std::map<location_id_t, state_t>::const_iterator symbolic_state_map_t<state_t>::end() const {
        return _states.end();
    }

    template<class state_t>
    bool symbolic_state_map_t<state_t>::equals(const symbolic_state_map_t<state_t>& rhs) const {
        if (this->size() != rhs.size())
            return false;

        return std::all_of(_states.begin(), _states.end(),
                           [&rhs](const std::pair<location_id_t, state_t>& s) {
            return rhs.at(s.first).equals(s.second); });
    }

    template<class state_t>
    void symbolic_state_map_t<state_t>::print(std::ostream& out, const TA& T) const {
        out << "Locations:\n";
        for (const auto& [loc, _] : _states)
            out << T.locations().at(loc).name() << "\n";

        out << "\nStates:\n";
        for (const auto& [_, s] : _states) {
            s.print(out, T);
            out << '\n';
        }
    }

    void concrete_state_t::delay(symb_time_t value) {
        auto d = value - _valuation[_valuation.size() - 1];
        if (d < 0)
            throw base_error("Error: Observed value <", value, "> is smaller than valuation of global clock <", _valuation[_valuation.size() - 1], ">");
        for (auto& v : _valuation)
            v += d;
        _valuation[0] = 0;
    }

    void concrete_state_t::delay(interval_t interval) {
        if (interval.first != interval.second)
            throw base_error("Error: Concrete states cannot delay in non-singular intervals. Use Interval monitor or delay monitor instead");
        delay(interval.first);
    }

    void concrete_state_t::free(const clocks_t& clocks) { return; }

    // Empty because we don't need restriction for concrete states.
    void concrete_state_t::restrict(const constraints_t &constraints) {
        if (!this->satisfies(constraints))
            set_empty();
    }

    void concrete_state_t::intersection(const symbolic_state_t& state) {
        if (!this->is_included_in(state))
            set_empty();
    }

    void concrete_state_t::intersection(const symbolic_state_map_t<symbolic_state_t>& states) {
        if (!this->is_included_in(states))
            set_empty();
    }

    bool concrete_state_t::do_transition(const edge_t &edge) {
        if (edge.from() != _location) {
            set_empty();
            return false;
        }
        // If the transition is not possible, do nothing and return false
        for (const auto& c : edge.guard())
            if (!satisfies(c)) {
                set_empty();
                return false;
            }

        _location = edge.to();

        for (const auto& r : edge.reset()) {
            _valuation[r] = 0;
        }

        return true;
    }

    valuation_t concrete_state_t::valuation() const {
        return _valuation;
    }

    location_id_t concrete_state_t::location() const {
        return _location;
    }

    bool concrete_state_t::is_empty() const {
        return _valuation[0] != 0;
    }

    bool concrete_state_t::is_included_in(const concrete_state_t& state) const {
        auto size = _valuation.size();
        if (state.valuation().size() != size)
            return false;
        if (this->is_empty())
            return state.is_empty();

        for (int i = 0; i < size; ++i) {
            if (_valuation[i] != state.valuation()[i])
                return false;
        }
        return true;
    }

    bool concrete_state_t::is_included_in(const symbolic_state_t &states) const {
        if (this->is_empty()) return true;

        if (states.location() != _location || states.is_empty()) {
            return false;
        } else {
            for (const auto &dbm : states.federation()) {
                bool sat = true;
                for (pardibaal::dim_t i = 0; i < dbm.dimension(); ++i)
                    for (pardibaal::dim_t j = 0; j < dbm.dimension(); ++j)
                       if (!satisfies(i, j, dbm.at(i, j)))
                            sat = false;
                if (sat) return true;
            }
        }

        return false;
    }

    bool concrete_state_t::is_included_in(const symbolic_state_map_t<symbolic_state_t> &states) const {
        if (this->is_empty()) return true;
        if (not states.has_state(_location)) {
            return false;
        }

        return is_included_in(states.at(_location));

    }

    bool concrete_state_t::satisfies(pardibaal::dim_t i, pardibaal::dim_t j, pardibaal::bound_t bound) const {
        if (this->is_empty()) 
            return false;
        if (bound.is_inf()) 
            return true;
        if (bound.is_strict()) {
            return ((zone_val_t) _valuation[i] - (zone_val_t) _valuation[j] < bound.get_bound());
        } else {
            return ((zone_val_t) _valuation[i] - (zone_val_t) _valuation[j] <= bound.get_bound());
        }
    }

    bool concrete_state_t::satisfies(const constraint_t &constraint) const {
        return satisfies(constraint._i, constraint._j, constraint._bound);
    }

    bool concrete_state_t::satisfies(const constraints_t &constraints) const {
        for (const auto& c : constraints)
            if (not this->satisfies(c))
                return false;

        return true;
    }

    concrete_state_t::concrete_state_t(location_id_t location, pardibaal::dim_t number_of_clocks) : _location(location) {
        _valuation = std::vector<concrete_time_t>(number_of_clocks + 1);
    }

    void concrete_state_t::set_empty() {
        _valuation[0] = 2;
    }
    
    void concrete_state_t::print(std::ostream& out, const TA& T) const {
        out << T.locations().at(this->_location).name() << " : ";
        auto max = _valuation.size() - 1;
        for (int i = 0; i < max; ++i)
            out << T.clock_name(i) << " = " << _valuation[i] << ", ";
        out << "global = " << _valuation[max] << '\n';
    }

    template struct symbolic_state_map_t<symbolic_state_t>;
    template struct symbolic_state_map_t<delay_state_t>;
    template struct symbolic_state_map_t<testing_state_t>;
}