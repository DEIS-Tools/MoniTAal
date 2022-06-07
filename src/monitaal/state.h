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

#include <map>
#include <vector>


namespace monitaal {

    //symbolic state
    struct symbolic_state_t {

        symbolic_state_t();
        symbolic_state_t(location_id_t location, const Federation &federation);

        void down();
        void restrict_to_zero(const clocks_t& clocks);
        void restrict(const constraints_t& constraints);
        void free(const clocks_t& clocks);
        void intersect(const symbolic_state_t& state);
        void add(const symbolic_state_t& state);
        void step_back(const edge_t& edge);

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] bool is_included_in(const symbolic_state_t& state) const;
        [[nodiscard]] bool equals(const symbolic_state_t& state) const;

        [[nodiscard]] location_id_t location_id() const;

        [[nodiscard]] Federation federation() const;

        void print(std::ostream& out, const TA& T) const;

    private:
        location_id_t _location;

        Federation _federation;
    };

    /**
     * states_map_t represents a set of symbolic states, stored in a map structure from locations to federations.
     */
    struct symbolic_state_map_t {
        void insert(symbolic_state_t state);
        void remove(location_id_t loc);

        [[nodiscard]] symbolic_state_t at(location_id_t loc) const;

        [[nodiscard]] symbolic_state_t& operator[](location_id_t loc);

        [[nodiscard]] bool is_empty() const;

        [[nodiscard]] size_t size() const;

        [[nodiscard]] bool has_state(location_id_t loc) const;

        void intersection(const symbolic_state_map_t& states);

        [[nodiscard]] std::map<location_id_t, symbolic_state_t>::iterator begin();
        [[nodiscard]] std::map<location_id_t, symbolic_state_t>::const_iterator begin() const;

        [[nodiscard]] std::map<location_id_t, symbolic_state_t>::iterator end();
        [[nodiscard]] std::map<location_id_t, symbolic_state_t>::const_iterator end() const;

        [[nodiscard]] bool equals(const symbolic_state_map_t& rhs) const;

        void print(std::ostream& out, const TA& T) const;


    private:
        std::map<location_id_t, symbolic_state_t> _states;
    };

    struct state_t {
        state_t(location_id_t location, pardibaal::dim_t number_of_clocks);

        void delay(value_t value);

        /**
         * Do a transition if possible. Returns false if the guard was not satisfied.
         * @param edge: The edge that defines the transition to be taken
         * @return True if the transition was possible and completed, false if not.
         */
        bool transition(const edge_t& edge);

        [[nodiscard]] valuation_t valuation() const;

        [[nodiscard]] location_id_t location() const;

        [[nodiscard]] bool is_included_in(const symbolic_state_map_t& states) const;

    private:
        location_id_t _location;
        valuation_t _valuation;
    };

}

#endif //MONITAAL_STATE_T_H
