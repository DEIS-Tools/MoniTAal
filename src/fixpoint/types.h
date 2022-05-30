/*
 * Copyright Thomas M. Grosen 
 * Created on 20/05/2022
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
#ifndef FIXPOINT_TYPES_H
#define FIXPOINT_TYPES_H

#include <pardibaal/Federation.h>

#include <vector>
#include <map>

namespace fixpoint {

    struct location_t;
    struct edge_t;
    struct state_t;

    using Federation = pardibaal::Federation;
    using Zone = pardibaal::DBM;
    using constraint_t = pardibaal::clock_constraint_t;

    using location_id_t = uint32_t;
    using clock_index_t = pardibaal::dim_t;
    using label_t       = std::string;

    using constraints_t = std::vector<constraint_t>;
    using clocks_t = std::vector<clock_index_t>;
    using edges_t = std::vector<edge_t>;
    using locations_t = std::vector<location_t>;

    using clock_map_t = std::map<clock_index_t, std::string>;
    using location_map_t = std::map<location_id_t, location_t>;
    using edge_map_t = std::map<location_id_t, edges_t>;

}

#endif //FIXPOINT_TYPES_H
