#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <shellapi.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iphlpapi.h>
#include <memory>
#include <iomanip>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Shell32.lib")

// Small console helper: prints one message to the console (wide output).
void print_line(const std::wstring& msg);

std::map<std::wstring, std::wstring> parse_args(const std::wstring& input);
std::wstring covert_ipv4_to_wstring(in_addr input);
std::wstring convert_phys_addr_to_wstring(BYTE phys_addr[], DWORD phys_addr_len);
DWORD convert_ip_addr_to_dword(std::wstring input);
DWORD get_interface_index_by_ip_addr(DWORD target_ip, PMIB_IPNETTABLE table);
DWORD get_interface_index_by_interface_ip(std::wstring target_ip, PMIB_IPADDRTABLE table);
std::wstring byte_to_hex(BYTE b);
std::vector<BYTE> wstring_to_phys_addr(std::wstring phys_addr);
