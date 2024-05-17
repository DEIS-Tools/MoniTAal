# Written by  ChatGPT on 10. april 2024 from prompts by Thomas Møller Grosen

import sys
import json
import xml.etree.ElementTree as ET
import math

def read_trace_file(trace_file_path):
    try:
        with open(trace_file_path, 'r') as f:
            json_data = json.load(f)
        return json_data
    except FileNotFoundError:
        print("Error: Trace file not found")
        sys.exit(1)
    except json.JSONDecodeError:
        print("Error: Invalid JSON format in trace file")
        sys.exit(1)

def read_model_file(model_file_path):
    try:
        tree = ET.parse(model_file_path)
        root = tree.getroot()
        return root
    except FileNotFoundError:
        print("Error: model file not found")
        sys.exit(1)
    except ET.ParseError:
        print("Error: Invalid XML format in model")
        sys.exit(1)

def time_eid_pairs(trace_data):
    #clutch = 0-3
    #gearbox = 4-7
    #gearcontrol = 8-33
    #interface = 34-57
    #engine = 58-69

    eid_map = {0: 8, 1: 34, 2: 58, 3: 4, 4: 0}

    pairs_list = []
    global_time = 0
    if "transitions" in trace_data:
        transitions = trace_data["transitions"]
        for transition in transitions:
            if "edges" in transition and transition["edges"]:
                first_edge = transition["edges"][0]
                if "parts" in first_edge and first_edge["parts"]:
                    first_part = first_edge["parts"][0]
                    if "eid" in first_part:
                        eid = first_part["eid"]
                        procnum = first_part["procnum"]
                        eid = eid_map[procnum] + eid
                        if "delay" in transition:
                            global_time += transition["delay"]
                            pairs_list.append((global_time, eid))
    return pairs_list

def eid_sync_map(model_data):
    pairs_dict = {}
    eid = 0
    for template in model_data.findall('.//template'):
        for transition in template.findall('.//transition'):
            transition_id = transition.get('id')[2:]
            transition_id = eid
            eid += 1
            label = transition.find(".//label[@kind='synchronisation']")
            value = label.text[:-1] if label is not None else ''
            pairs_dict[int(transition_id)] = value

    return pairs_dict

def write_observations(observations, sync_map, out_path):
    out = open(out_path, "w")

    max = 0

    for time, eid in observations:
        if (eid > max):
            max = eid
        out.write("@" + str(math.floor(time)) + " " + sync_map[eid] + "\n")
    print(max)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python uctr-to-monpoly.py <path_to_trace_file> <path_to_model_file> <path_to_output_file>")
        sys.exit(1)
    
    trace_file_path = sys.argv[1]
    model_file_path = sys.argv[2]
    output_file_path = sys.argv[3]

    trace_data = read_trace_file(trace_file_path)
    model_data = read_model_file(model_file_path)

    observations = time_eid_pairs(trace_data)
    sync_map = eid_sync_map(model_data)

    write_observations(observations, sync_map, output_file_path)