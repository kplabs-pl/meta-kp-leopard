import tomllib
import os
import sys

option_sheet_path = sys.argv[1]

with open(option_sheet_path, "rb") as f:
    option_sheet = tomllib.load(f)

csp_version = int(option_sheet['leopard']['interface']['csp_version'])

processing_node_iface = option_sheet['leopard']['interface']['processing_node']
csp_address = int(processing_node_iface['csp_address'])
netmask_length = int(processing_node_iface['csp_netmask'])

host_id_length = 14 if csp_version == 2 else 5

netmask = ((1 << netmask_length) - 1) << (host_id_length - netmask_length)

network_address = csp_address & netmask
print(network_address)
