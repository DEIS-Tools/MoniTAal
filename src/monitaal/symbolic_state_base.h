/*
 * Copyright Thomas M. Grosen 
 * Created on 06/04/2024
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

#ifndef MONITAAL_SYMBOLIC_STATE_BASE
#define MONITAAL_SYMBOLIC_STATE_BASE

#include "types.h"
#include "TA.h"

namespace monitaal {

    struct symbolic_state_base {

        symbolic_state_base();
        symbolic_state_base(location_id_t location, clock_index_t clocks);

        void down();

        void restrict_to_zero(const clocks_t& clocks);

        void restrict(const constraints_t& constraints);

        void free(const clocks_t& clocks);

        void intersection(const symbolic_state_base& state);

        void add(const symbolic_state_base& state);

        bool do_transition(const edge_t& edge);
        void do_transition_backward(const edge_t& edge);

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] relation_t relation(const symbolic_state_base& state) const;
        [[nodiscard]] bool is_included_in(const symbolic_state_base& state) const;
        [[nodiscard]] bool equals(const symbolic_state_base& state) const;

        [[nodiscard]] bool satisfies(const constraint_t& constraint) const;
        [[nodiscard]] bool satisfies(const constraints_t& constraints) const;

        [[nodiscard]] location_id_t location() const;

        [[nodiscard]] Federation federation() const;

        void print(std::ostream& out, const TA& T) const;
    
    protected:
        location_id_t _location;

        Federation _federation;
    };
}

#endif //MONITAAL_SYMBOLIC_STATE_BASE