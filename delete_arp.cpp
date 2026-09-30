#include "delete_arp.h"


void delete_ip_net_entry(const std::wstring& args)
{
	print_line(L"[*] Delete Arp Entry\n");
	DWORD ip_as_dword;
	DWORD status;
	MIB_IPNETROW arp_entry{};
	std::unique_ptr<BYTE[]> ip_net_table_buffer;
	std::unique_ptr<BYTE[]> ip_addr_table_buffer;
	PMIB_IPNETTABLE ip_net_table;
	PMIB_IPADDRTABLE ip_addr_table;
	std::wstring ip_addr;
	std::wstring ip_interface;
	std::map<std::wstring, std::wstring> options = parse_args(args);


	if (options.find(L"ip") != options.end())
	{
		ip_addr = options[L"ip"];
	}

	if (options.find(L"if") != options.end())
	{
		ip_interface = options[L"if"];
	}
	WSADATA wsaData;
	int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsa_result != 0)
	{
		print_line(L"[-] Winsock initialize failed\n");
		goto FINAL;
	}

	if (ip_addr.empty())
	{
		print_line(L"[-] Bad argument empty ip address\n");
		goto FINAL;
	}

	ip_as_dword = convert_ip_addr_to_dword(ip_addr);
	if (ip_as_dword == INADDR_NONE)
	{
		print_line(L"[-] Bad argument ip address, " + ip_addr + L"\n");
		goto FINAL;
	}
	ip_net_table_buffer = get_ip_net_table();
	ip_net_table = reinterpret_cast<PMIB_IPNETTABLE>(ip_net_table_buffer.get());

	if (ip_net_table_buffer == nullptr)
	{
		print_line(L"[-] Couldn't find a interface number to add your arp entry\n");
		goto FINAL;
	}

	// try search ip interface using ip net table
	if (ip_interface.empty())
	{
		arp_entry.dwIndex = get_interface_index_by_ip_addr(ip_as_dword, ip_net_table);
		if (arp_entry.dwIndex == DWORD(-1))
		{
			print_line(L"[-] Cann't find interface contains the specified IP address.\n");
			goto FINAL;
		}
	}
	else
	{
		ip_addr_table_buffer = get_ip_addr_table();
		ip_addr_table = reinterpret_cast<PMIB_IPADDRTABLE>(ip_addr_table_buffer.get());
		if (ip_addr_table == NULL)
		{
			print_line(L"[-] Cann't get Ip address table.\n");
			goto FINAL;
		}
		arp_entry.dwIndex = get_interface_index_by_interface_ip(ip_interface, ip_addr_table);
		if (arp_entry.dwIndex == DWORD(-1))
		{
			print_line(L"[-] Invalid Interface.\n");
			goto FINAL;
		}
	}



	arp_entry.dwAddr = ip_as_dword;
	status = DeleteIpNetEntry(&arp_entry);
	if (status == ERROR_ACCESS_DENIED)
	{
		std::wstring tmp = L"[-] Couldn't delete " + ip_addr + L", error: " + std::to_wstring(status) + L"\n";
		tmp += L"[-] ARP: The requested operation requires elevation.\n";
		print_line(tmp);
		goto FINAL;
	}
	if (status != ERROR_SUCCESS)
	{
		std::wstring tmp = L"[-] Couldn't delete " + ip_addr + L", error: " + std::to_wstring(status) + L"\n";
		print_line(tmp);
	}


FINAL:
	WSACleanup();
	print_line(L"[*] Final Delete IP Net Arp Entry\n");
}
