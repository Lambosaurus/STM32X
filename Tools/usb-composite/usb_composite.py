
import os
from preprocessor import Preprocessor
from string import Template
import re
import argparse

LIB_PATH = "../../Lib"
MCU_TYPE = "STM32L0" # Actually doesnt matter, as long as its valid

# The template for the setup request handling. Each class must be triggered on the appropriate interface
SETUP_REQUEST_TMPL = Template("""\
    ${if_clause} (interface < ${interface_max})
    {
        ${class_setup};
    }\
""")

CONFIG_DESCRIPTOR_SIZE = 9
IAD_DESCRIPTOR_SIZE = 8

class USBClassInfo():
    def __init__(self, name, interface_base, endpoint_base):
        self.name = name.upper()
        name_lower = name.lower()

        self.src_path = "usb/{}/USB_{}.c".format(name_lower, self.name)
        self.hdr_path = "usb/{}/USB_{}.h".format(name_lower, self.name)

        self.interface_base = interface_base
        self.endpoint_base = endpoint_base
        self.defines = {
            "USB_{}_INTERFACE_BASE".format(self.name): interface_base,
            "USB_{}_ENDPOINT_BASE".format(self.name): endpoint_base,
        }

        self.includes = [ self.hdr_path ]
        self.iad_required = False

    def parse(self):
        self.parse_definitions()
        self.parse_descriptor()

    def parse_definitions(self):
        p = Preprocessor()

        # Ignore missing files, and USB_Composite files.
        p.add_include_path(LIB_PATH)
        p.ignore_missing_includes = True
        p.include_rule = lambda x: "USB_Composite." not in x
        p.define(MCU_TYPE)
        p.define("USE_USB")
        p.define("USB_CLASS_{}".format(self.name))

        # Include the USB_Class header, and use this to collect the class definitons
        p.include("usb/USB_Class.h")

        self.interfaces = p.evaluate("USB_INTERFACES")
        self.endpoints = p.evaluate("USB_ENDPOINTS") - 1 # Exclude the control endpoint

        self.class_init = p.expand("USB_CLASS_INIT(config);")
        self.class_deinit = p.expand("USB_CLASS_DEINIT();")
        self.class_setup = p.expand("USB_CLASS_SETUP(req);")

        self.config_descriptor_name = p.expand("USB_CONFIG_DESCRIPTOR")

        if self.interfaces > 1 and p.is_defined("USB_CLASS_CLASSID"):
            self.iad_required = True

            self.class_id = p.evaluate("USB_CLASS_CLASSID")
            self.subclass_id = p.evaluate("USB_CLASS_SUBCLASSID")
            self.protocol_id = p.evaluate("USB_CLASS_PROTOCOLID")


    def parse_descriptor(self):
        p = Preprocessor()

        # Include nothing external. This will cause the descriptor to be mostly unexpanded
        p.add_include_path(LIB_PATH)
        p.ignore_missing_includes = True
        p.include_rule = lambda x: self.name in x
        p.define(MCU_TYPE)
        p.define("USE_USB")
        p.define("USB_CLASS_{}".format(self.name))
        for key in self.defines: p.define( key, self.defines[key] )

        # Read the source
        p.include(self.src_path)
        src = p.source()
        
        # Extract the descriptor body
        restring = self.config_descriptor_name + r"\s*\[\s*(\w+)\s*\]\s*=\s*\{([^\}]+)\}"
        match = re.search(restring, src)
        size, body = match.groups()
        descriptors = [e for e in self.split_expressions(body)]

        # Remove the configuration descriptor - as the USB composite class will handle it
        config_block = descriptors.pop(0)
        assert("USB_DESCR_BLOCK_CONFIGURATION" in config_block) # Sanity check
        size = p.evaluate(size) - CONFIG_DESCRIPTOR_SIZE

        if self.iad_required:
            # Add an interface association descriptor to combine the interfaces
            descriptors.insert(0, "USB_DESC_BLOCK_INTERFACE_ASSOCIATION({}, {}, {}, {}, {})".format(
                self.interface_base, self.interfaces, self.class_id, self.subclass_id, self.protocol_id
            ))
            size += IAD_DESCRIPTOR_SIZE

        # Add a comment to identify the class
        descriptors.insert(0, "// USB_{}".format(self.name))

        self.config_body = self.format_descriptor(descriptors)
        self.config_size = size

    # Takes in the expression list from a class descriptor
    # Returns a tidyer list of expressions
    def format_descriptor(self, exprs):
        line = ""
        lines = []
        for expr in exprs:
            expr += ','
            if len(line) == 0 or (len(line) + len(expr) <= (6*4)):
                line += expr
            else:
                lines.append(line)
                line = expr
        if len(line):
            lines.append(line)
        return lines

    # Splits a string into a list of expressions
    # Expressions are separated by commas, and may be enclosed in brackets
    def split_expressions(self, text):
        expr = ""
        brackets = 0
        whitespace = 1
        for ch in text:
            if ch == '(':
                brackets += 1
            elif ch == ')':
                brackets -= 1
            elif ch in "\n\t":
                ch = ' '
            elif ch == ',' and brackets == 0:
                yield expr
                expr = ""
                continue
            if ch == ' ':
                if whitespace > 0:
                    continue
                whitespace += 1
            expr += ch
        if len(expr):
            yield expr


# Flattens a nested list into a single list
def flatten(sources):
    for source in sources:
        for item in source:
            yield item

# Joins a list of strings into a single string, with newlines and indentation
def join_and_tab(items, padding = "\t"):
    return "\n".join(padding + item for item in items)

# Generated the output from a template file and the substitutions
def substitute_file_template(template_file, output_file, substitutions):
    with open(template_file, "r") as f:
        template = Template(f.read())
    text = template.substitute(**substitutions)
    with open(output_file, "w") as f:
        f.write(text)

# Generates the source and header for a USB composite device
def generate_composite_device(usb_classes, dest_folder):
    interfaces = 0
    endpoints = 1 # Control endpoint
    
    config_size = 0
    config_body = []
    setup_requests = []

    # Add the configuration descrptor for usb composite
    config_body.append("USB_DESCR_BLOCK_CONFIGURATION(USB_COMPOSITE_CONFIG_DESC_SIZE, USB_COMPOSITE_INTERFACES, 0x01),")
    config_size += CONFIG_DESCRIPTOR_SIZE

    # Iterate over the classes, parsing their content
    infos = []
    for usb_class in usb_classes:
        info = USBClassInfo(usb_class, interfaces, endpoints)
        info.parse()

        interfaces += info.endpoints
        endpoints += info.interfaces
        config_size += info.config_size
        config_body.extend(info.config_body)

        setup_requests.append(SETUP_REQUEST_TMPL.substitute(
            interface_max = interfaces,
            class_setup = info.class_setup,
            if_clause = "else if" if len(infos) else "if"
        ))

        infos.append(info)

    iad_required = any(info.iad_required for info in infos)

    # Create the output files
    os.makedirs(dest_folder, exist_ok=True)
    substitute_file_template("templates/USB_Composite.c", os.path.join(dest_folder, "USB_Composite.c"), {
        "class_init": join_and_tab(info.class_init for info in infos),
        "class_deinit": join_and_tab(info.class_deinit for info in infos),
        "class_setup": "\n".join(setup_requests),
        "class_descriptor": join_and_tab(config_body),
        "class_includes": "\n".join( '#include "{}"'.format(h) for h in flatten(info.includes for info in infos)),
    })
    substitute_file_template("templates/USB_Composite.h", os.path.join(dest_folder, "USB_Composite.h"), {
        "interfaces": interfaces,
        "endpoints": endpoints,
        "descriptor_size": config_size,
        "definitions": "\n".join( '#define {}\t\t\t\t{}'.format(k,v) for k,v in flatten(info.defines.items() for info in infos) ),
        "class_id": "0xEF" if iad_required else "0x00",
        "subclass_id": "0x02" if iad_required else "0x00",
        "protocol_id":"0x01" if iad_required else "0x00",
    })


if __name__ == "__main__":
    # Example usage: python usb_composite.py --dest "../../../user" --classes CDC MSC 

    parser = argparse.ArgumentParser(description="Generate USB composite device")
    parser.add_argument("--dest", help="Destination folder", default="../../../user")
    parser.add_argument("--classes", help="USB class to generate", nargs="+", default=[])
    args = parser.parse_args()

    print("Generating USB composite device for classes {}".format(args.classes))
    generate_composite_device(args.classes, args.dest)
    print("Done")
