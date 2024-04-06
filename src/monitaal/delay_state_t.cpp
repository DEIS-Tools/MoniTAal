/*
 * Copyright Thomas M. Grosen 
 * Created on 05/04/2024
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

#include "delay_state_t.h"

namespace monitaal {

    delay_state_t::delay_state_t() : _jitter(0) {}

    delay_state_t::delay_state_t(location_id_t location, clock_index_t clocks, interval_t latency, symb_time_t jitter) : 
            symbolic_state_base(location, clocks + 2), _jitter(jitter) {
        _etime = clocks;
        _time = clocks + 1;
        
        _federation.free(_etime);
        _federation.restrict(_time, _etime, pardibaal::bound_t::non_strict(latency.first));
        _federation.restrict(_etime, _time, pardibaal::bound_t::non_strict(latency.second));
    }

    delay_state_t delay_state_t::unconstrained(location_id_t location, clock_index_t clocks) {
        auto state = delay_state_t();

        state._location = location;
        state._federation = Federation::unconstrained(clocks + 2);
        // state._jitter = (symb_time_t) -1;
        return state;
    }

    void delay_state_t::intersection(const symbolic_state_base& state) {
        symbolic_state_base::intersection(state);
    }

    void delay_state_t::intersection(const symbolic_state_map_t& map) {
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

    bool delay_state_t::is_included_in(const symbolic_state_map_t &map) const {
        if (not map.has_state(_location))
            return false;

        return is_included_in(map.at(_location));
    }

    bool delay_state_t::is_included_in(const symbolic_state_base& state) const {
        return symbolic_state_base::is_included_in(state);
    }

}