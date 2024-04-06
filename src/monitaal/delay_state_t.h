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

#ifndef MONITAAL_DELAY_STATE_H
#define MONITAAL_DELAY_STATE_H

#include "types.h"
#include "TA.h"
#include "state.h"


#include <vector>
namespace monitaal {

    struct delay_state_t : public symbolic_state_base {
        delay_state_t();
        delay_state_t(location_id_t location, clock_index_t clocks, interval_t latency, symb_time_t jitter);

        [[nodiscard]] static delay_state_t unconstrained(location_id_t location, clock_index_t clocks);

        void intersection(const symbolic_state_base& state);
        void intersection(const symbolic_state_map_t& map);

        void delay(symb_time_t value);
        void delay(interval_t interval);

        [[nodiscard]] bool is_included_in(const symbolic_state_base& state) const;
        [[nodiscard]] bool is_included_in(const symbolic_state_map_t& map) const;

    private:
        clock_index_t _etime, _time;
        symb_time_t _jitter;
    };

}

#endif //MONITAAL_DELAY_STATE_H