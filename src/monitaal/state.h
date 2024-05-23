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

#ifndef MONITAAL_STATE_T_H
#define MONITAAL_STATE_T_H

#include "types.h"
#include "TA.h"
#include "symbolic_state_base.h"

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>

#include <map>
#include <vector>


namespace monitaal {

    template<class state_t>
    class state_map_t;

    //symbolic state
    struct symbolic_state_t : public symbolic_state_base {

        symbolic_state_t();
        symbolic_state_t(location_id_t location, clock_index_t clocks);

        [[nodiscard]] static symbolic_state_t unconstrained(location_id_t location, clock_index_t clocks);

        void intersection(const state_map_t<symbolic_state_t>& map);
        void intersection(const symbolic_state_base& state);

        void delay(symb_time_t value);
        void delay(interval_t interval);

        [[nodiscard]] bool is_included_in(const state_map_t<symbolic_state_t>& map) const;
        [[nodiscard]] bool is_included_in(const symbolic_state_base& state) const;
    };

    struct delay_state_t : public symbolic_state_base {
        delay_state_t();
        delay_state_t(location_id_t location, clock_index_t clocks, interval_t latency, symb_time_t jitter);

        [[nodiscard]] static delay_state_t unconstrained(location_id_t location, clock_index_t clocks);

        void intersection(const symbolic_state_base& state);
        void intersection(const state_map_t<delay_state_t>& map);

        void delay(symb_time_t value);
        void delay(interval_t interval);

        [[nodiscard]] boost::icl::interval_set<symb_time_t> get_latency() const;
        [[nodiscard]] symb_time_t get_jitter_bound() const;

        [[nodiscard]] bool is_included_in(const symbolic_state_base& state) const;
        [[nodiscard]] bool is_included_in(const state_map_t<delay_state_t>& map) const;

    private:
        clock_index_t _etime, _time;
        symb_time_t _jitter;
    };

    /**
     * states_map_t represents a set of symbolic states, stored in a map structure from locations to federations.
     */
    template<class state_t>
    struct state_map_t {
        void insert(state_t state);
        void remove(location_id_t loc);

        [[nodiscard]] state_t at(location_id_t loc) const;

        [[nodiscard]] state_t& operator[](location_id_t loc);

        [[nodiscard]] bool is_empty() const;

        [[nodiscard]] size_t size() const;

        [[nodiscard]] bool has_state(location_id_t loc) const;

        void intersection(const state_map_t& states);

        [[nodiscard]] std::map<location_id_t, state_t>::iterator begin();
        [[nodiscard]] std::map<location_id_t, state_t>::const_iterator begin() const;

        [[nodiscard]] std::map<location_id_t, state_t>::iterator end();
        [[nodiscard]] std::map<location_id_t, state_t>::const_iterator end() const;

        [[nodiscard]] bool equals(const state_map_t<state_t>& rhs) const;

        void print(std::ostream& out, const TA& T) const;


    private:
        std::map<location_id_t, state_t> _states;
    };

    struct concrete_state_t {
        concrete_state_t(location_id_t location, pardibaal::dim_t number_of_clocks);

        void delay(symb_time_t value);
        void delay(interval_t interval);

        // Small hack: This is used in the monitor template, but only relevant for symbolic states.
        // Therefore this is just an empty implementation.
        void restrict(const constraints_t& constraints);

        void intersection(const symbolic_state_t& state);
        void intersection(const state_map_t<symbolic_state_t>& states);

        /**
         * Do a transition if possible. Returns false if the guard was not satisfied.
         * @param edge: The edge that defines the transition to be taken
         * @return True if the transition was possible and completed, false if not.
         */
        bool do_transition(const edge_t& edge);

        [[nodiscard]] valuation_t valuation() const;

        [[nodiscard]] location_id_t location() const;

        [[nodiscard]] bool is_empty() const;

        [[nodiscard]] bool is_included_in(const concrete_state_t& state) const;

        [[nodiscard]] bool is_included_in(const symbolic_state_t& states) const;

        [[nodiscard]] bool is_included_in(const state_map_t<symbolic_state_t>& states) const;

        [[nodiscard]] bool satisfies(pardibaal::dim_t i, pardibaal::dim_t j, pardibaal::bound_t bound) const;
        [[nodiscard]] bool satisfies(const constraint_t& constraint) const;
        [[nodiscard]] bool satisfies(const constraints_t& constraints) const;

        void print(std::ostream& out, const TA& T) const;

    private:
        void set_empty();
        
        location_id_t _location;
        valuation_t _valuation;
    };
}

#endif //MONITAAL_STATE_T_H
