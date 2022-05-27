/*
 * Copyright Thomas M. Grosen 
 * Created on 23/05/2022
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
#ifndef FIXPOINT_FIXPOINT_H
#define FIXPOINT_FIXPOINT_H

#include "TA.h"
#include "types.h"
#include "state_t.h"

namespace fixpoint {

    class Fixpoint {
    public:
        static states_map_t reach(const states_map_t& states, const TA& T);
        static states_map_t accept_states(const TA& T);
        static states_map_t buchi_accept_fixpoint(const TA& T);
    };

}

#endif //FIXPOINT_FIXPOINT_H
