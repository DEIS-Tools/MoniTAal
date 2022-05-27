/*
 * Copyright Thomas M. Grosen 
 * Created on 24/05/2022
 */

/*
 * This file is part of fixpoint
 *
 * fixpoint is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fixpoint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with fixpoint. If not, see <https://www.gnu.org/licenses/>.
 */
#include "state_t.h"
#include "types.h"
#include "TA.h"

#include <pardibaal/Federation.h>
#include <iostream>
#include <algorithm>

namespace fixpoint {

    state_t::state_t() {
        _location = 0;
        _federation = Federation();
    }

    state_t::state_t(location_id_t location, const Federation& federation) : _location(location) {
        _federation = federation;
    }

    void state_t::down() {
        _federation.past();
    }

    void state_t::restrict_to_zero(const clocks_t& clocks) {
        for (const auto& x : clocks) {
            _federation.restrict(x, 0, pardibaal::bound_t::non_strict(0));
        }
    }

    void state_t::restrict(const constraints_t& constraints) {
        for (const auto& c : constraints)
            _federation.restrict(c);
    }

    void state_t::free(const clocks_t& clocks) {
        for (const auto& x : clocks)
            _federation.free(x);
    }

    void state_t::intersect(const state_t& state) {
        if (state._location == _location)
            _federation.intersection(state._federation);
    }

    void state_t::add(const state_t& state) {
        if (state.location_id() == _location)
            _federation.add(state._federation);
    }

    void state_t::step_back(const edge_t& edge) {

        if (edge.to() == _location) {
            _location = edge.from();

            this->down();
            this->restrict_to_zero(edge.reset());
            this->free(edge.reset());
            this->restrict(edge.guard());
            this->down();
        }
    }

    bool state_t::is_empty() const {
        return _federation.is_empty();
    }

    bool state_t::is_included_in(const state_t &state) const {
        if (state._location == _location)
            return _federation.intersects(state._federation);
        return false;
    }

    bool state_t::equals(const state_t& state) const {
        return _federation.equal(state._federation);
    }

    location_id_t state_t::location_id() const {
        return _location;
    }

    void state_t::print(std::ostream &out, const TA &T) const {
        out << T.locations().at(_location).name() << ' ' << _federation;
    }


    void states_map_t::insert(state_t state) {

        if (not state.is_empty()) {
            if (_states.find(state.location_id()) == _states.end()) {
                _states.insert({state.location_id(), state});
            } else {
                _states[state.location_id()].add(state);
            }
        }
    }

    void states_map_t::remove(location_id_t loc) {
        _states.erase(loc);
    }

    state_t states_map_t::at(location_id_t loc) const {
        return _states.at(loc);
    }

    state_t &states_map_t::operator[](location_id_t loc) {
        return _states[loc];
    }

    bool states_map_t::is_empty() const {
        return _states.empty();
    }

    size_t states_map_t::size() const {
        return _states.size();
    }

    bool states_map_t::has_state(location_id_t loc) const {
        return _states.find(loc) != _states.end();
    }

    void states_map_t::intersection(const states_map_t& states) {
        std::vector<location_id_t> erase_list;

        for(const auto& [l, _] : this->_states) {
            if (states.has_state(l)) {
                _states[l].intersect(states.at(l));
            } else {
                erase_list.push_back(l);
            }
        }

        for (const auto& l : erase_list)
            remove(l);
    }

    std::map<location_id_t, state_t>::iterator states_map_t::begin() {
        return _states.begin();
    }

    std::map<location_id_t, state_t>::const_iterator states_map_t::begin() const {
        return _states.begin();
    }

    std::map<location_id_t, state_t>::iterator states_map_t::end() {
        return _states.end();
    }

    std::map<location_id_t, state_t>::const_iterator states_map_t::end() const {
        return _states.end();
    }

    bool states_map_t::equals(const states_map_t& rhs) const {
        if (this->size() != rhs.size())
            return false;

        return std::all_of(_states.begin(), _states.end(),
                           [&rhs](const std::pair<location_id_t, state_t>& s) {
            return rhs.at(s.first).equals(s.second); });
    }

    void states_map_t::print(std::ostream& out, const TA& T) const {
        out << "Locations:\n";
        for (const auto& [loc, _] : _states)
            out << T.locations().at(loc).name() << "\n";

        out << "\nStates:\n";
        for (const auto& [_, s] : _states) {
            s.print(out, T);
            out << '\n';
        }
    }

}