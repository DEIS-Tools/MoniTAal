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

#include "types.h"
#include "TA.h"

#include <utility>
#include <iostream>

namespace monitaal {

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

    edge_t::edge_t(location_id_t from, location_id_t to, const constraints_t& guard, const clocks_t& reset, const label_t& label) :
            _from(from), _to(to), _guard(guard), _reset(reset), _label(label) {}

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
            _name(std::move(name)), _number_of_clocks(clocks.size()), _clock_names(clocks), _initial(initial) {
        location_map_t loc_map;
        edge_map_t backward_edges, forward_edges;

        for (const auto &l : locations) {
            loc_map.insert({l.id(), l});

            edges_t to, from;
            for (const auto &e : edges) {
                _labels.insert(e.label());
                if (e.to() == l.id())
                    to.push_back(e);
                if (e.from() == l.id())
                    from.push_back(e);
            }
            backward_edges.insert({l.id(), to});
            forward_edges.insert({l.id(), from});

        }

        _locations = std::move(loc_map);
        _backward_edges = std::move(backward_edges);
        _forward_edges = std::move(forward_edges);
    }

    const edges_t& TA::edges_to(location_id_t id) const { return _backward_edges.at(id); }

    const edges_t& TA::edges_from(location_id_t id) const { return _forward_edges.at(id); }

    std::string TA::clock_name(clock_index_t index) const { return _clock_names.at(index); }

    const location_map_t &TA::locations() const { return _locations; }

    location_id_t TA::initial_location() const { return _initial; }

    clock_index_t TA::number_of_clocks() const { return _number_of_clocks; }

    const std::unordered_set<label_t>& TA::labels() const { return _labels;}

    void TA::intersection(const TA &other) {

        clock_map_t new_clocks;

        new_clocks.insert({0, "0"});
        for (const auto& [index, name] : this->_clock_names) {
            if (index != 0) {
                new_clocks.insert({index, name + "_1"});
            }
        }

        // Assume clock indexes range from 0 and onwards. Append them all (except for 0 clock, hence the - 1)
        auto clock_size = this->_clock_names.size() - 1;
        for (const auto& [index, name] : other._clock_names) {
            if (index != 0) {
                new_clocks.insert({index + clock_size, name + "_2"});
            }
        }

        locations_t new_locations;
        std::map<std::pair<location_id_t, location_id_t>, std::pair<location_id_t, location_id_t>> new_loc_indir;
        location_id_t tmp_id = 0;
        for (const auto& [id1, loc1] : this->_locations) {
            for (const auto& [id2, loc2] : other.locations()) {
                new_loc_indir.insert({{loc1.id(), loc2.id()}, {tmp_id, tmp_id+1}});

                constraints_t constr(loc1.invariant());
                for (const auto& c : loc2.invariant()) {
                    constr.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_size),
                                                  (c._j == 0 ? 0 : c._j + clock_size), c._bound));
                }

                location_t new_loc1(false, tmp_id, loc1.name() + '_' + loc2.name() + "_1", constr);
                location_t new_loc2(loc2.is_accept(), tmp_id+1, loc1.name() + '_' + loc2.name() + "_2", constr);

                new_locations.push_back(new_loc1);
                new_locations.push_back(new_loc2);
                tmp_id += 2;
            }
        }

        edges_t new_edges;
        for (const auto& [_, vec1] : this->_forward_edges)
            for (const auto& e1 : vec1)
                // Add loops when label is not in alphabet of other
                if (not other.labels().contains(e1.label())) {
                    for (const auto& [l, _] : other.locations()) {
                        const auto& [from_l1, from_l2] = new_loc_indir.at({e1.from(), l});
                        const auto& [to_l1, to_l2] = new_loc_indir.at({e1.to(), l});
                        
                        edge_t new_e1(from_l1, (this->locations().at(e1.from()).is_accept() 
                                                ? to_l2 : to_l1), e1.guard(), e1.reset(), e1.label());

                        edge_t new_e2(from_l2, (other.locations().at(l).is_accept() 
                                                ? to_l1 : to_l2), e1.guard(), e1.reset(), e1.label());

                        new_edges.push_back(new_e1);
                        new_edges.push_back(new_e2);
                    }
                }
                else 
                    for (const auto& [_, vec2] : other._forward_edges)
                        for (const auto& e2 : vec2)
                            if (not e1.label().compare(e2.label())) {

                                constraints_t guard(e1.guard());
                                for (const auto& c : e2.guard()) {
                                    guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_size),
                                                                (c._j == 0 ? 0 : c._j + clock_size), c._bound));
                                }

                                clocks_t reset(e1.reset());
                                for (const auto& r : e2.reset())
                                    reset.push_back(r == 0 ? 0 : r + clock_size);

                                const std::string label = e1.label();

                                const auto& [to_l1, to_l2] = new_loc_indir.at({e1.to(), e2.to()});
                                const auto& [from_l1, from_l2] = new_loc_indir.at({e1.from(), e2.from()});

                                edge_t new_e1(from_l1, (this->locations().at(e1.from()).is_accept() 
                                                        ? to_l2 : to_l1), guard, reset, label);

                                edge_t new_e2(from_l2, (other.locations().at(e2.from()).is_accept() 
                                                        ? to_l1 : to_l2), guard, reset, label);

                                new_edges.push_back(new_e1);
                                new_edges.push_back(new_e2);
                            }
        
        // Add loops when label is not in alphabet of this
        for (const auto& [_, vec2] : other._forward_edges)
            for (const auto& e2 : vec2)
                if (not this->labels().contains(e2.label())) {
                    for (const auto& [l, _] : this->locations()) {
                        const auto& [from_l1, from_l2] = new_loc_indir.at({l, e2.from()});
                        const auto& [to_l1, to_l2] = new_loc_indir.at({l, e2.to()});

                        constraints_t guard;
                        for (const auto& c : e2.guard()) {
                            guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_size),
                                                        (c._j == 0 ? 0 : c._j + clock_size), c._bound));
                        }

                        clocks_t reset;
                        for (const auto& r : e2.reset())
                            reset.push_back(r == 0 ? 0 : r + clock_size);
                        
                        edge_t new_e1(from_l1, (this->locations().at(l).is_accept() 
                                                ? to_l2 : to_l1), guard, reset, e2.label());

                        edge_t new_e2(from_l2, (other.locations().at(e2.from()).is_accept() 
                                                ? to_l1 : to_l2), guard, reset, e2.label());

                        new_edges.push_back(new_e1);
                        new_edges.push_back(new_e2);
                    }
                }


        *this = TA(this->_name + '_' + other._name, new_clocks, new_locations, new_edges,
                   new_loc_indir[{this->initial_location(), other.initial_location()}].first);
        
        // Add labels from other to this
        auto tmp_labels = other.labels();
        this->_labels.merge(tmp_labels);

    }

    TA TA::time_divergence_ta(const std::vector<std::string>& alphabet, bool deterministic) {
        clock_map_t clocks;
        clocks.insert({0, "0"});
        clocks.insert({1, "div_clock"});

        locations_t locations;
        locations.push_back(location_t(true, 0, "time_div1", constraints_t{}));
        locations.push_back(location_t(false, 1, "time_div2", constraints_t{}));

        edges_t edges;
        for (const auto& a : alphabet) {
            if (deterministic)
                edges.push_back(edge_t(1, 1, constraints_t{constraint_t::upper_strict(1, 1)}, clocks_t{}, a));
            else
                edges.push_back(edge_t(1, 1, constraints_t{}, clocks_t{}, a));
            edges.push_back(edge_t(0, 1, constraints_t{}, clocks_t{1}, a));
            edges.push_back(edge_t(1, 0, constraints_t{constraint_t::lower_non_strict(1, 1)}, clocks_t{}, a));
        }

        return TA("time_divergence", clocks, locations, edges, 0);
    }

    void TA::print_dot(std::ostream& out) const {
        std::string colours[4] = {"purple", "red", "black", "green2"};
        int col_count = 0;
        int col_max = 4;

        out << "digraph \"" << _name << "\" {";
        
        out << "\tinit [label=\"\", shape=\"none\"]; init -> " << _initial << ";\n\n";
        for (const auto& [id, loc] : _locations) {

            out << "\t" << id << " [label=\"" << loc.name();
            if (!loc.invariant().empty()) {
                out << "\\n";
                print_constraint(out, loc.invariant());
            }
            out << "\", shape=\"";
            if (loc.is_accept())
                out << "doublecircle\"";
            else
                out << "circle\"";
            out << "];\n";
        }

        out << "\n";

        for (const auto& [l_id, edges] : this->_forward_edges) {
            for (const auto& e : edges) {
                out << "\t" << l_id << " -> " << e.to() << " [label=\"" << e.label();

                if (!e.guard().empty()) {
                    out << "\\n";
                    this->print_constraint(out, e.guard());
                }
                
                if (!e.reset().empty()) {
                    out << "\\n";
                    bool first = true;
                    for (const auto& x : e.reset()) {
                        if (first) first = false;
                        else out << ", ";
                        out << this->clock_name(x) << " := 0";
                    }
                }

                out << "\", color=\"" << colours[col_count] << "\", fontcolor=\"" << colours[col_count] << "\"];\n";
                col_count = (col_count + 1) % col_max;
            }
        }
        out << "}";
    }

    std::ostream& operator<<(std::ostream& out, const TA& T) {
        out << T._name << "\n  Locations: (" << T._locations.at(T._initial).name() << ")\n";
        for (const auto& [_,loc] : T._locations) {
            out << "\n    " << loc.name();
            if (loc.is_accept()) out << " (accept)";

            if (!loc.invariant().empty()) {
                out << " invariant: ";
                T.print_constraint(out, loc.invariant());
            }
        }

        out << "\n  Edges:\n";
        for (const auto& es : T._backward_edges) {
            for (const auto& e : es.second) {
                out << "    " << T._locations.at(e.from()).name() << " -> " << T._locations.at(e.to()).name();
                
                out << " [" << e.label() << ']';

                if (!e.reset().empty()) {
                    out << " reset: ";
                    for (const auto& x : e.reset())
                        out << T._clock_names.at(x) << ", ";
                }
                
                if (!e.guard().empty()) {
                    out << " guard: ";
                    T.print_constraint(out, e.guard());
                }
                out << "\n";
            }
        }

        return out;
    }

    void TA::print_constraint(std::ostream& out, const constraints_t& constraints) const {
        bool first = true;
        for (const auto& c : constraints) {
            if (first) first = false;
            else out << " & ";
            if (c._i != 0) out << this->clock_name(c._i) << ' ';
            if (c._j != 0) out << "- " << this->clock_name(c._j) << ' ';
            out << (c._bound.is_strict() ? "< " : "<= ") << c._bound.get_bound();
        }
    }
}