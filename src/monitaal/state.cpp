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

#include <pardibaal/Federation.h>
#include <iostream>
#include <algorithm>

namespace monitaal {

    symbolic_state_t::symbolic_state_t() {
        _location = 0;
        _federation = Federation();
    }

    symbolic_state_t::symbolic_state_t(location_id_t location, const Federation& federation) : _location(location) {
        _federation = federation;
    }

    void symbolic_state_t::down() {
        _federation.past();
    }

    void symbolic_state_t::restrict_to_zero(const clocks_t& clocks) {
        for (const auto& x : clocks) {
            _federation.restrict(x, 0, pardibaal::bound_t::non_strict(0));
        }
    }

    void symbolic_state_t::restrict(const constraints_t& constraints) {
        for (const auto& c : constraints)
            _federation.restrict(c);
    }

    void symbolic_state_t::free(const clocks_t& clocks) {
        for (const auto& x : clocks)
            _federation.free(x);
    }

    void symbolic_state_t::intersect(const symbolic_state_t& state) {
        if (state._location == _location)
            _federation.intersection(state._federation);
    }

    void symbolic_state_t::add(const symbolic_state_t& state) {
        if (state.location() == _location)
            _federation.add(state._federation);
    }

    void symbolic_state_t::delay(symb_time_t value) {
        _federation.delay(((pardibaal::val_t) value));
    }

    void symbolic_state_t::delay(interval_t interval) {
        _federation.interval_delay((pardibaal::val_t) interval.first, (pardibaal::val_t) interval.second);
    }

    bool symbolic_state_t::do_transition(const edge_t& edge) {
        if (edge.from() != _location) return false;

        // If the transition is not possible, do nothing and return false
        if (not this->satisfies(edge.guard()))
            return false;

        if (!edge.guard().empty()) {
            for (auto& c : edge.guard())
                _federation.restrict(c);
        }

        for (const auto& r : edge.reset())
            _federation.assign(r, 0);

        _location = edge.to();
        return true;
    }

    void symbolic_state_t::do_transition_backward(const edge_t& edge) {

        if (edge.to() == _location) {
            _location = edge.from();

            this->down();
            this->restrict_to_zero(edge.reset());
            this->free(edge.reset());
            this->restrict(edge.guard());
            this->down();
        }
    }

    bool symbolic_state_t::is_empty() const {
        return _federation.is_empty();
    }

    bool symbolic_state_t::is_included_in(const symbolic_state_map_t &states) const {
        if (not states.has_state(_location))
            return false;

        return is_included_in(states.at(_location));
    }

    bool symbolic_state_t::is_included_in(const symbolic_state_t &state) const {
        if (state._location == _location) {
            auto rel = _federation.approx_relation(state._federation);
            return rel.is_equal() || rel.is_subset();
        }
        return false;
    }

    bool symbolic_state_t::equals(const symbolic_state_t& state) const {
        return _federation.is_approx_equal(state._federation);
    }

    bool symbolic_state_t::satisfies(const constraint_t &constraint) const {
        return _federation.is_satisfying(constraint);
    }

    bool symbolic_state_t::satisfies(const constraints_t &constraints) const {
        return _federation.is_satisfying(constraints);
    }

    location_id_t symbolic_state_t::location() const {
        return _location;
    }

    Federation symbolic_state_t::federation() const { return _federation; }

    void symbolic_state_t::print(std::ostream &out, const TA &T) const {
        out << T.locations().at(_location).name() << ' ' << _federation;
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
                this->_states[l].intersect(states.at(l));
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
        for (auto& v : _valuation)
            v += value;
        _valuation[0] = 0;
    }

    // Empty because we don't need restriction for concrete states.
    void concrete_state_t::restrict(const constraints_t &constraints) {
        if (!this->satisfies(constraints))
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
        _valuation = std::vector<concrete_time_t>(number_of_clocks);
    }
}