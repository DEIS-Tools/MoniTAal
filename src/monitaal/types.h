/*
 * Copyright Thomas M. Grosen 
 * Created on 20/05/2022
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

#ifndef MONITAAL_TYPES_H
#define MONITAAL_TYPES_H

#include <pardibaal/Federation.h>

#include <cinttypes>
#include <vector>
#include <map>

namespace monitaal {

    struct location_t;
    struct edge_t;
    struct symbolic_state_t;

    template<bool is_interval> struct timed_input_t;
    template<bool is_interval> class Monitor;

    using Federation = pardibaal::Federation;
    using Zone       = pardibaal::DBM;

    using zone_val_t = pardibaal::val_t;

    using constraint_t  = pardibaal::difference_bound_t;
    using constraints_t = std::vector<constraint_t>;

    using clock_index_t = pardibaal::dim_t;
    using clock_map_t   = std::map<clock_index_t, std::string>;
    using clocks_t      = std::vector<clock_index_t>;

    using location_id_t  = uint32_t;
    using location_map_t = std::map<location_id_t, location_t>;
    using locations_t    = std::vector<location_t>;

    using edges_t    = std::vector<edge_t>;
    using edge_map_t = std::map<location_id_t, edges_t>;

    using label_t = std::string;

    using symb_time_t = uint32_t;
    using interval_t = std::pair<symb_time_t, symb_time_t>;

    using concrete_time_t = float;
    using valuation_t = std::vector<concrete_time_t>;

    using interval_input = timed_input_t<true>;
    using concrete_input = timed_input_t<false>;

    using Interval_monitor = Monitor<true>;
    using Concrete_monitor = Monitor<false>;

}

#endif //MONITAAL_TYPES_H
