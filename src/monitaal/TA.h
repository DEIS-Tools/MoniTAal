/*
 * Copyright Thomas M. Grosen 
 * Created on 19/05/2022
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

#ifndef MONITAAL_TA_H
#define MONITAAL_TA_H

#include "types.h"

#include <pardibaal/DBM.h>
#include <map>
#include <unordered_set>

namespace monitaal {

    struct location_t {
        location_t(bool accept, location_id_t id, std::string name, constraints_t invariant);

        [[nodiscard]] bool is_accept() const;

        [[nodiscard]] location_id_t id() const;

        [[nodiscard]] std::string name() const;

        [[nodiscard]] constraints_t invariant() const;

        [[nodiscard]] Zone invariant_zone(clock_index_t dimension) const;

    private:
        const bool _accept;
        const location_id_t _id;
        const std::string _name;
        const constraints_t _invariant;
    };

    struct edge_t {
        edge_t(location_id_t from, location_id_t to, const constraints_t& guard, const clocks_t& reset, const label_t& label);

        [[nodiscard]] location_id_t from() const;

        [[nodiscard]] location_id_t to() const;

        [[nodiscard]] const constraints_t &guard() const;

        [[nodiscard]] const clocks_t &reset() const;

        [[nodiscard]] Zone guard_zone(clock_index_t dimension) const;

        [[nodiscard]] label_t label() const;

    private:
        const location_id_t _from, _to;

        const constraints_t _guard;

        const clocks_t _reset;

        const label_t _label;
    };

    class TA {

        std::string _name;

        clock_map_t _clock_names;

        location_map_t _locations;

        edge_map_t _backward_edges;

        edge_map_t _forward_edges;

        location_id_t _initial;

        clock_index_t _number_of_clocks;

        std::unordered_set<label_t> _labels = std::unordered_set<label_t>();

        void print_constraint(std::ostream& out, const constraints_t& constraints) const;

    public:

        TA(std::string name, clock_map_t clocks, const locations_t &locations, const edges_t &edges, location_id_t initial);

        [[nodiscard]] const edges_t &edges_to(location_id_t id) const;

        [[nodiscard]] const edges_t &edges_from(location_id_t id) const;

        [[nodiscard]] std::string clock_name(clock_index_t index) const;

        [[nodiscard]] const location_map_t& locations() const;

        [[nodiscard]] location_id_t initial_location() const;

        [[nodiscard]] clock_index_t number_of_clocks() const;

        [[nodiscard]] const std::unordered_set<label_t>& labels() const;

        void intersection (const TA& other);

        static TA time_divergence_ta(const std::vector<std::string>& alphabet, bool deterministic);

        void print_dot(std::ostream& out) const;

        friend std::ostream& operator<<(std::ostream& out, const TA& T);
    };

    std::ostream& operator<<(std::ostream& out, const TA& T);
}

#endif //MONITAAL_TA_H
