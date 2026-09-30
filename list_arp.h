#pragma once
#include "utils.h"

std::unique_ptr<BYTE[]> get_ip_net_table();
std::unique_ptr<BYTE[]> get_ip_addr_table();
void print_ip_net_table();
std::wstring interface_idx_to_ip(PMIB_IPADDRTABLE arp_table, DWORD index);
