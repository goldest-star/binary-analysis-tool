import json
import sys
import math
import importlib
import glob
import os

from kaitaistruct import KaitaiStruct

#import pprint
#pp = pprint.PrettyPrinter(indent=2)

def dump_struct(s, sections, prefix="", parent_offset = 0, root_io=None, root_offset=0):
    if isinstance(s, list):
        #print(f"\n[BEGIN List Under '{prefix}']")
        for i, item in enumerate(s):
            label = prefix + "[" + str(i) + "]"
            #print(label)
            sections.append({
                "label": label,
                "parent": prefix
            })
            dump_struct(item, sections, label, parent_offset, root_io, root_offset)
        #print(f"[END OF List Under '{prefix}']\n")
    elif isinstance(s, KaitaiStruct):
        #pp.pprint(vars(s))
        if hasattr(s, "_debug"):

            section_io = s._io
            if root_io is None:
                root_io = section_io
            elif section_io is not root_io:
                root_io = section_io
                root_offset = root_offset + parent_offset

            #print("\nITEMS {\n")
            for name, descr in s._debug.items():
                prop = getattr(s, name)
                label = prefix + "." + name if prefix else name
                #print(f"(\n  name: {label},\n  descr: {descr},\n  prop: {prop},\n  offset: {root_offset}\n)\n")

                sections.append({
                    "start": descr["start"] + root_offset,
                    "end": descr["end"] + root_offset,
                    "label": label,
                    "parent": prefix
                })


                parent_offset = descr["start"] + root_offset

                dump_struct(prop, sections, label, parent_offset, root_io, root_offset)

            #print("}\n")

def parse_data(input_filename, output_filename, action_progress):
    # locate the compiled struct module
    scripts = glob.glob(os.path.join(os.path.dirname(input_filename), '*.py'))
    if len(scripts) < 1:
        raise FileNotFoundError('Could not find the expected python kaitai parser - did the kaitai struct compiler fail?')
    module_file = os.path.basename(scripts[0])
    sys.path.append(os.path.dirname(scripts[0]))
    package_name = os.path.splitext(module_file)[0]
    class_name = package_name.capitalize()
    struct_module = importlib.__import__(package_name, fromlist=[class_name])
    Struct = getattr(struct_module, class_name)

    action_progress.set_progress_percent(5)

    # parse the input data
    target = Struct.from_file(input_filename)
    target._read()

    action_progress.set_progress_percent(70)

    # write the parser result to the output
    sections = []

    dump_struct(target, sections)

    #pp.pprint(sections)

    action_progress.set_progress_percent(80)

    output_json = {
    "sections": sections
    }

    with open(output_filename, 'w') as output_file:
        json.dump(output_json, output_file)
