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
#include "Parser.h"

#include <string>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string/predicate.hpp>

namespace fixpoint {

    TA Parser::parse(const char *path) {
        pugi::xml_document doc;
        if (not load_file(doc, path)) {
            std::cerr << "Error: Failed to load model file " << path << '\n';
            exit(-1);
        }

        auto xml_ta = doc.child("nta").child("template");

        pugi::xml_text ta_name = xml_ta.child("name").text();
        std::vector<std::string> ta_clocks;
        ta_clocks.emplace_back("0");

        if (ta_name.empty())
            throw std::exception();

        // locations
        locations_t locations;
        for (pugi::xml_node loc = xml_ta.child("location"); not loc.empty(); loc = loc.next_sibling("location")) {
            location_id_t id = parse_loc_id(loc.attribute("id").as_string());

            std::string name = loc.child("name").text().as_string();

            constraints_t invariant = parse_constraint(loc.child("label").text().as_string(), ta_clocks);

            locations.push_back(location_t(boost::algorithm::ends_with(name, "_a"), id, name, invariant));
        }

        // transitions
        edges_t edges;
        for (pugi::xml_node tran = xml_ta.child("transition"); not tran.empty(); tran = tran.next_sibling("transition")) {
            location_id_t from = parse_loc_id(tran.child("source").attribute("ref").as_string()),
                            to = parse_loc_id(tran.child("target").attribute("ref").as_string());

            auto reset_node = tran.find_child([](pugi::xml_node node){ return not std::strcmp(node.attribute("kind").as_string(), "assignment"); });
            clocks_t reset = parse_reset(reset_node.text().as_string(), ta_clocks);

            auto guard_node = tran.find_child([](pugi::xml_node node){ return std::strcmp(node.attribute("kind").as_string(), "guard") == 0; });
            constraints_t guard = parse_constraint(guard_node.text().as_string(), ta_clocks);

            edges.push_back(edge_t(from, to, guard, reset));
        }

        clock_map_t clocks;

        for (clock_index_t i = 0; i < ta_clocks.size(); ++i)
            clocks.insert({i, ta_clocks[i]});

        location_id_t initial = parse_loc_id(xml_ta.child("init").attribute("ref").as_string());

        return TA(ta_name.as_string(), clocks, locations, edges, initial);
    }

    bool Parser::load_file(pugi::xml_document &doc, const char *path) {
        pugi::xml_parse_result result = doc.load_file(path);

        if (result) {
            std::cout << "Parsed \"" << path << "\" without errors\n";
            return true;
        }
        else {
            std::cout << "Parsed \"" << path << "\" with errors: " << result.description() << '\n';
            std::cout << "Error offset: " << result.offset << '\n';
            return false;
        }
    }

    constraints_t Parser::parse_constraint(const std::string& input, std::vector<std::string>& clocks) {
        constraints_t constraints;

        std::istringstream stream(input);
        std::string word;

        while (stream >> word) {
            if (std::strcmp(word.c_str(), "&&") == 0)
                stream >> word;

            clock_index_t x = parse_clock(word, clocks);

            stream >> word;
            operator_e op = parse_operator(word);

            stream >> word;
            pardibaal::val_t bound = std::stoi(word);

            switch (op) {
                case EQ:
                    constraints.push_back(constraint_t::lower_non_strict(x, bound));
                    constraints.push_back(constraint_t::upper_non_strict(x, bound));
                    break;
                case G:  constraints.push_back(constraint_t::lower_strict(x, bound));     break;
                case GE: constraints.push_back(constraint_t::lower_non_strict(x, bound)); break;
                case L:  constraints.push_back(constraint_t::upper_strict(x, bound));     break;
                case LE: constraints.push_back(constraint_t::upper_non_strict(x, bound)); break;
            }
        }

        return constraints;
    }

    clocks_t Parser::parse_reset(const std::string &input, std::vector<std::string>& clocks) {
        clocks_t reset;

        std::istringstream stream(input);
        std::string word;

        while (stream >> word) {
            reset.push_back(parse_clock(word, clocks));
            //<clock> <:=> <number>, ...
            //skip operator and number (we assume it is 0)
            stream >> word; stream >> word;
        }

        return reset;
    }

    location_id_t Parser::parse_loc_id(const char *input) {
        return std::stoi(input + 2);
    }

    clock_index_t Parser::parse_clock(const std::string& clock, std::vector<std::string>& clocks) {
        if (not clock_exists(clock, clocks)) {
            clocks.push_back(clock);
            return clocks.size() - 1;
        } else {
            return get_index(clock, clocks);
        }

    }

    bool Parser::clock_exists(const std::string& clock, const std::vector<std::string>& clocks) {
        for (clock_index_t i = 0; i < clocks.size(); ++i) {
            if (std::strcmp(clocks[i].c_str(), clock.c_str()) == 0)
                return true;
        }
        return false;
    }

    clock_index_t Parser::get_index(const std::string& clock, const std::vector<std::string>& clocks) {
        for (clock_index_t i = 0; i < clocks.size(); ++i) {
            if (std::strcmp(clocks[i].c_str(), clock.c_str()) == 0)
                return i;
        }
        return 0;
    }

    operator_e Parser::parse_operator(const std::string &op) {
        if (std::strcmp(op.c_str(), "==") == 0) return EQ;
        if (std::strcmp(op.c_str(), ">") == 0) return G;
        if (std::strcmp(op.c_str(), ">=") == 0) return GE;
        if (std::strcmp(op.c_str(), "<") == 0) return L;
        if (std::strcmp(op.c_str(), "<=") == 0) return LE;
        else return ASSIGN;
    }
}
