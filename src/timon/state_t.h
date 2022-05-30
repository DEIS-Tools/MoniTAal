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
#ifndef FIXPOINT_STATE_T_H
#define FIXPOINT_STATE_T_H

#include "types.h"
#include "TA.h"

#include <map>
#include <vector>


namespace timon {

    //symbolic state
    struct state_t {

        state_t();
        state_t(location_id_t location, const Federation &federation);

        void down();
        void restrict_to_zero(const clocks_t& clocks);
        void restrict(const constraints_t& constraints);
        void free(const clocks_t& clocks);
        void intersect(const state_t& state);
        void add(const state_t& state);
        void step_back(const edge_t& edge);

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] bool is_included_in(const state_t& state) const;
        [[nodiscard]] bool equals(const state_t& state) const;

        [[nodiscard]] location_id_t location_id() const;

        void print(std::ostream& out, const TA& T) const;

    private:
        location_id_t _location;

        Federation _federation;
    };

    struct states_map_t {
        void insert(state_t state);
        void remove(location_id_t loc);

        [[nodiscard]] state_t at(location_id_t loc) const;

        [[nodiscard]] state_t& operator[](location_id_t loc);

        [[nodiscard]] bool is_empty() const;

        [[nodiscard]] size_t size() const;

        [[nodiscard]] bool has_state(location_id_t loc) const;

        void intersection(const states_map_t& states);

        [[nodiscard]] std::map<location_id_t, state_t>::iterator begin();
        [[nodiscard]] std::map<location_id_t, state_t>::const_iterator begin() const;

        [[nodiscard]] std::map<location_id_t, state_t>::iterator end();
        [[nodiscard]] std::map<location_id_t, state_t>::const_iterator end() const;

        [[nodiscard]] bool equals(const states_map_t& rhs) const;

        void print(std::ostream& out, const TA& T) const;


    private:
        std::map<location_id_t, state_t> _states;
    };

}

#endif //FIXPOINT_STATE_T_H
