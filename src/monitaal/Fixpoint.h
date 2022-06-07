/*
 * Copyright Thomas M. Grosen 
 * Created on 23/05/2022
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

#ifndef MONITAAL_MONITAAL_H
#define MONITAAL_MONITAAL_H

#include "TA.h"
#include "types.h"
#include "state.h"

namespace monitaal {

    class Fixpoint {
    public:
        static symbolic_state_map_t reach(const symbolic_state_map_t& states, const TA& T);
        static symbolic_state_map_t accept_states(const TA& T);
        static symbolic_state_map_t buchi_accept_fixpoint(const TA& T);
    };

}

#endif //MONITAAL_MONITAAL_H
