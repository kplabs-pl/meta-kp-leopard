import tomllib
import os
import sys

option_sheet_path = sys.argv[1]

with open(option_sheet_path, "rb") as f:
    option_sheet = tomllib.load(f)

csp_version = int(option_sheet['leopard']['interface']['csp_version'])
host_id_length = 14 if csp_version == 2 else 5

def make_netmask(netmask_length):
    return ((1 << netmask_length) - 1) << (host_id_length - netmask_length)


def make_hostmask(netmask_length):
    return (1 << (host_id_length - netmask_length)) - 1


processing_node_iface = option_sheet['leopard']['interface']['processing_node']
svr_pn_csp_address = int(processing_node_iface['csp_address'])
svr_pn_csp_netmask_length = int(processing_node_iface['csp_netmask'])

svr_pn_netmask = make_netmask(svr_pn_csp_netmask_length)

svr_pn_network_address = svr_pn_csp_address & svr_pn_netmask

notify_to = []

for iface in ['can_a', 'can_b', 'uart_a', 'uart_b']:
    if iface not in option_sheet['leopard']['interface']:
        continue

    iface_csp_adress = int(option_sheet['leopard']['interface'][iface]['csp_address'])
    iface_csp_netmask_length = int(option_sheet['leopard']['interface'][iface]['csp_netmask'])
    iface_csp_netmask = make_netmask(iface_csp_netmask_length)
    iface_network_address = iface_csp_adress & iface_csp_netmask
    iface_broadcast_address = iface_network_address | make_hostmask(iface_csp_netmask_length)
    notify_to.append(iface_broadcast_address)


print({
    'svr_pn_csp_address': svr_pn_csp_address,
    'svr_pn_csp_netmask_length': svr_pn_csp_netmask_length,
    'csp_version': csp_version,
    'host_id_length': host_id_length,
    'netmask': svr_pn_netmask,
    'network_address': svr_pn_network_address,
}, file=sys.stderr)

env_vars = {
    "COMMUNICATION_HUB_RUNTIME_VARIABLE_DIR": "/var/run/leopard/communication-hub",
    "COMM_HUB_INPUT_ADDRESS": "ipc:///var/run/leopard/communication-hub/input",
    "COMM_HUB_BROADCAST_ADDRESS": "ipc:///var/run/leopard/communication-hub/broadcast",
    "COMM_HUB_COLLECTOR_ADDRESS": "ipc:///var/run/leopard/communication-hub/collector",
    "COMM_HUB_SPI_DEVICE": "/dev/leopard/spi_slave",
    "SERVICE_CSP_NETMASK_FILE": "/var/run/leopard-pn-netmask",
    "SERVICE_JOB_RUNNER_WORKING_DIRECTORY": "/opt/leopard/jobs",
    "SERVICE_CSP_VERSION": csp_version,
    "SERVICE_CSP_NETMASK_LENGTH": svr_pn_csp_netmask_length,
    "SERVICE_PN1_NETWORK_ADDRESS": svr_pn_network_address,
    "SERVICE_NOTIFY_TO": " ".join(str(addr) for addr in notify_to),
    "SERVICE_FILESYSTEM_CSP_ADDRESS": 1,
    "SERVICE_JOB_RUNNER_CSP_ADDRESS": 2,
}

for name, value in env_vars.items():
    print(f"{name}={value}")
