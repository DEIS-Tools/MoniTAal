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
namespace timon {

    location_t::location_t(bool accept, location_id_t id, std::string name, constraints_t invariant) :
            _accept(accept), _id(id), _name(std::move(name)), _invariant(std::move(invariant)) {}

    bool location_t::is_accept() const { return _accept; }

    location_id_t location_t::id() const { return _id; }

    std::string location_t::name() const { return _name; }

    constraints_t location_t::invariant() const { return _invariant; }

    Zone location_t::invariant_zone(clock_index_t dimension) const {
        auto rtn = Zone::unconstrained(dimension);

        for (const auto &c : _invariant)
            rtn.restrict(c);

        return rtn;
    }

    edge_t::edge_t(location_id_t from, location_id_t to, constraints_t& guard, clocks_t& reset, label_t& label) :
            _from(from), _to(to), _guard(std::move(guard)), _reset(std::move(reset)), _label(std::move(label)) {}

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

    label_t edge_t::label() const {return _label;}

    TA::TA(std::string name, clock_map_t clocks, const locations_t &locations, const edges_t &edges, location_id_t initial) :
            _name(std::move(name)), number_of_clocks(clocks.size()), _clock_names(clocks), _initial(initial) {
        location_map_t loc_map;
        edge_map_t edge_map;

        for (const auto &l : locations) {
            loc_map.insert({l.id(), l});

            edges_t tmp;
            for (const auto &e : edges) {
                if (e.to() == l.id())
                    tmp.push_back(e);
            }
            edge_map.insert({l.id(), tmp});
        }

        _locations = std::move(loc_map);
        _backward_edges = std::move(edge_map);
    }

    const edges_t &TA::edges_to(location_id_t id) const { return _backward_edges.at(id); }

    std::string TA::clock_name(clock_index_t index) const { return _clock_names.at(index); }

    const location_map_t &TA::locations() const { return _locations; }

    location_id_t TA::initial_location() const { return _initial; }

    std::ostream& operator<<(std::ostream& out, const TA& T) {
        out << T._name << "\n  Locations: (" << T._locations.at(T._initial).name() << ")";
        for (const auto& loc : T._locations) {
            out << "\n    " << loc.second.name() << " ";
            if (loc.second.is_accept()) out << "(accept) ";

            out << "invariant: ";
            for (const auto& c : loc.second.invariant()) {
                if (c._i != 0) out << T._clock_names.at(c._i) << ' ';
                if (c._j != 0) out << "- " << T._clock_names.at(c._j) << ' ';
                out << (c._bound.is_strict() ? "< " : "<= ") << c._bound.get_bound() << ", ";
            }
        }

        out << "\n  Edges:";
        for (const auto& es : T._backward_edges) {
            for (const auto& e : es.second) {
                out << "    " << T._locations.at(e.from()).name() << " -> " << T._locations.at(e.to()).name();
                out << ": reset: ";
                for (const auto& x : e.reset())
                    out << T._clock_names.at(x) << ", ";
                out << " guard: ";
                for (const auto& g : e.guard()) {
                    if (g._i != 0) out << T._clock_names.at(g._i) << ' ';
                    if (g._j != 0) out << "- " << T._clock_names.at(g._j) << ' ';
                    out << (g._bound.is_strict() ? "< " : "<= ") << g._bound.get_bound() << ", ";
                }
                out << " label: " << e.label() << '\n';

            }
        }

        return out;
    }
}