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

    void symbolic_state_t::intersection(const symbolic_state_map_t& map) {
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

    bool symbolic_state_t::is_included_in(const symbolic_state_map_t& map) const {
        if (not map.has_state(_location))
            return false;

        return is_included_in(map.at(_location));
    }

    bool symbolic_state_t::is_included_in(const symbolic_state_base &state) const {
        return symbolic_state_base::is_included_in(state);
    }

    void symbolic_state_map_t::insert(symbolic_state_t state) {

        if (not state.is_empty()) {
            if (not this->has_state(state.location())) {
                _states.insert({state.location(), state});
            } else {
                _states[state.location()].add(state);
            }
        }
    }

    void symbolic_state_map_t::remove(location_id_t loc) {
        _states.erase(loc);
    }

    symbolic_state_t symbolic_state_map_t::at(location_id_t loc) const {
        return _states.at(loc);
    }

    symbolic_state_t &symbolic_state_map_t::operator[](location_id_t loc) {
        return _states[loc];
    }

    bool symbolic_state_map_t::is_empty() const {
        return _states.empty();
    }

    size_t symbolic_state_map_t::size() const {
        return _states.size();
    }

    bool symbolic_state_map_t::has_state(location_id_t loc) const {
        return _states.find(loc) != _states.end();
    }

    void symbolic_state_map_t::intersection(const symbolic_state_map_t& states) {
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

    std::map<location_id_t, symbolic_state_t>::iterator symbolic_state_map_t::begin() {
        return _states.begin();
    }

    std::map<location_id_t, symbolic_state_t>::const_iterator symbolic_state_map_t::begin() const {
        return _states.begin();
    }

    std::map<location_id_t, symbolic_state_t>::iterator symbolic_state_map_t::end() {
        return _states.end();
    }

    std::map<location_id_t, symbolic_state_t>::const_iterator symbolic_state_map_t::end() const {
        return _states.end();
    }

    bool symbolic_state_map_t::equals(const symbolic_state_map_t& rhs) const {
        if (this->size() != rhs.size())
            return false;

        return std::all_of(_states.begin(), _states.end(),
                           [&rhs](const std::pair<location_id_t, symbolic_state_t>& s) {
            return rhs.at(s.first).equals(s.second); });
    }

    void symbolic_state_map_t::print(std::ostream& out, const TA& T) const {
        out << "Locations:\n";
        for (const auto& [loc, _] : _states)
            out << T.locations().at(loc).name() << "\n";

        out << "\nStates:\n";
        for (const auto& [_, s] : _states) {
            s.print(out, T);
            out << '\n';
        }
    }

    void concrete_state_t::delay(concrete_time_t value) {
        auto d = value - _valuation[_valuation.size() - 1];
        if (d < 0)
            throw base_error("Error: Observed value <", value, "> is smaller than valuation of global clock <", _valuation[_valuation.size() - 1], ">");
        for (auto& v : _valuation)
            v += d;
        _valuation[0] = 0;
    }

    // Empty because we don't need restriction for concrete states.
    void concrete_state_t::restrict(const constraints_t &constraints) {
        if (!this->satisfies(constraints))
            _valuation[0] = -1;
    }

    void concrete_state_t::intersection(const symbolic_state_t& state) {
        if (!this->is_included_in(state))
            _valuation[0] = -1;
    }

    void concrete_state_t::intersection(const symbolic_state_map_t& states) {
        if (!this->is_included_in(states))
            _valuation[0] = -1;
    }

    bool concrete_state_t::do_transition(const edge_t &edge) {
        if (edge.from() != _location) {
            _valuation[0] = -1; // if the first is -1 then the valuation is empty/invalid
            return false;
        }
        // If the transition is not possible, do nothing and return false
        for (const auto& c : edge.guard())
            if (!satisfies(c)) {
                _valuation[0] = -1;
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
        return _valuation[0] < 0;
    }

    bool concrete_state_t::is_included_in(const concrete_state_t& state) const {
        auto size = _valuation.size();
        if (state.valuation().size() != size)
            return false;

        for (int i = 0; i < size; ++i) {
            if (_valuation[i] != state.valuation()[i])
                return false;
        }
        return true;
    }

    bool concrete_state_t::is_included_in(const symbolic_state_t &states) const {
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

    bool concrete_state_t::is_included_in(const symbolic_state_map_t &states) const {
        if (not states.has_state(_location)) {
            return false;
        }

        return is_included_in(states.at(_location));

    }

    bool concrete_state_t::satisfies(pardibaal::dim_t i, pardibaal::dim_t j, pardibaal::bound_t bound) const {
        if (_valuation[0] < 0) 
            return false;
        if (bound.is_inf()) 
            return true;
        if (bound.is_strict()) {
            return (_valuation[i] - _valuation[j] < bound.get_bound());
        } else {
            return (_valuation[i] - _valuation[j] <= bound.get_bound());
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
    
    void concrete_state_t::print(std::ostream& out, const TA& T) const {
        out << T.locations().at(this->_location).name() << " : ";
        auto max = _valuation.size() - 1;
        for (int i = 0; i < max; ++i)
            out << T.clock_name(i) << " = " << _valuation[i] << ", ";
        out << "global = " << _valuation[max] << '\n';
    }
}