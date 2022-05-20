/*
 * Copyright Thomas M. Grosen 
 * Created on 19/05/2022
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

#include "types.h"
#include "TA.h"

#include <utility>
namespace fixpoint {

    location_t::location_t(bool accept, location_id_t id, std::string name, constraints_t invariant) :
            _accept(accept), _id(id), _name(std::move(name)), _invariant(std::move(invariant)) {}

    bool location_t::is_accept() const { return _accept; }

    location_id_t location_t::id() const { return _id; }

    std::string location_t::name() const { return _name; }

    constraints_t location_t::invariant() const { return _invariant; }

    Zone location_t::invariant_zone(clock_index_t dimension) const {
        auto rtn = pardibaal::DBM::unconstrained(dimension);

        for (const auto &c : _invariant)
            rtn.restrict(c);

        return rtn;
    }

    edge_t::edge_t(location_id_t from, location_id_t to, constraints_t guard, clocks_t reset) :
            _from(from), _to(to), _guard(std::move(guard)), _reset(std::move(reset)) {}

    location_id_t edge_t::from() const {
        return _from;
    }

    location_id_t edge_t::to() const {
        return _to;
    }

    const clocks_t &edge_t::reset() const {
        return _reset;
    }

    const constraints_t &edge_t::guard() const {
        return _guard;
    }

    Zone edge_t::guard_zone(clock_index_t dimension) const {
        auto rtn = Zone::unconstrained(dimension);

        for (const auto &c : _guard)
            rtn.restrict(c);

        return rtn;
    }

    TA::TA(clock_map_t clocks, const std::vector<location_t> &locations, const edges_t &edges) :
            _clock_names(std::move(clocks)), number_of_clocks(clocks.size()) {
        location_map_t loc_map;
        edge_map_t edge_map;

        for (const auto &l : locations) {
            loc_map.insert({l.id(), l});

            edges_t tmp;
            for (const auto &e : edges) {
                if (e.to() == l.id())
                    tmp.push_back(e);

                edge_map.insert({l.id(), tmp});
            }
        }

        _locations = std::move(loc_map);
        _backward_edges = std::move(edge_map);
    }

    const edges_t &TA::edges_to(location_id_t id) const {
        return _backward_edges.at(id);
    }

    std::string TA::clock_name(clock_index_t index) const {
        return _clock_names.at(index);
    }
}