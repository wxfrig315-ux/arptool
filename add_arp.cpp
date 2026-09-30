#include "add_arp.h"
#include "list_arp.h"

void set_ip_net_entry(const std::wstring& args)
{
	print_line(L"[*] Adds ARP Entry \n");
	DWORD ip_addr_dword;
	std::wstringstream logger;
	std::vector<BYTE> phys_addr_buffer;
	std::unique_ptr<BYTE[]> ip_addr_table_buffer;
	PMIB_IPADDRTABLE ip_addr_table;
	MIB_IPNETROW arp_entry{};
	DWORD interface_idx{};
	DWORD status;
	WSADATA wsaData;

	std::wstring ip_addr;
	std::wstring physic_addr;
	std::wstring interface_addr;

	std::map<std::wstring, std::wstring> options = parse_args(args);

	if (options.find(L"ip") != options.end())
	{
		ip_addr = options[L"ip"];
	}

	if (options.find(L"if") != options.end())
	{
		interface_addr = options[L"if"];
	}

	if (options.find(L"mac") != options.end())
	{
		physic_addr = options[L"mac"];
	}

	int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsa_result != 0)
	{
		print_line(L"[-] Winsock initialized failed\n");
		goto FINAL;
	}


	if (ip_addr.empty() || physic_addr.empty())
	{
		print_line(L"[-] Bad missing Agrument!\n");
		goto FINAL;
	}

	ip_addr_dword = convert_ip_addr_to_dword(ip_addr);
	if (ip_addr_dword == INADDR_NONE)
	{
		logger << L"[-] Bad Agrument: " << ip_addr << std::endl;
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}

	phys_addr_buffer = wstring_to_phys_addr(physic_addr);
	if (physic_addr.empty() || phys_addr_buffer.size() != 6)
	{
		logger << L"[-] Bad Agrument: " << physic_addr << std::endl;
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}
	print_line(L"[+] Retrieve ip adress table...\n");
	ip_addr_table_buffer = get_ip_addr_table();
	ip_addr_table = reinterpret_cast<PMIB_IPADDRTABLE>(ip_addr_table_buffer.get());

	print_line(L"[+] Find interface index\n");
	if (interface_addr.empty())
	{
		for (size_t i = 0; i < ip_addr_table->dwNumEntries; i++)
		{
			if (ip_addr_table->table[i].dwAddr != convert_ip_addr_to_dword(L"127.0.0.1"))
			{
				interface_idx = ip_addr_table->table[i].dwIndex;
			}
		}

	}
	else
	{
		interface_idx = get_interface_index_by_interface_ip(interface_addr, ip_addr_table);
	}

	if (interface_idx == DWORD(-1) || ip_addr_table == NULL)
	{
		logger << L"[-] Failed to get index: " << interface_addr << std::endl;
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}

	arp_entry.dwAddr = ip_addr_dword;
	arp_entry.Type = MIB_IPNET_TYPE_STATIC;
	arp_entry.dwIndex = interface_idx;
	arp_entry.dwPhysAddrLen = 6;
	memcpy(arp_entry.bPhysAddr, phys_addr_buffer.data(), 6);

	status = CreateIpNetEntry(&arp_entry);

	if (status == ERROR_ACCESS_DENIED)
	{
		logger << L"[-] Couldn't add " << ip_addr << L" " << interface_addr << L" " << physic_addr << L" ,error: " << std::to_wstring(status) << std::endl;
		logger << L"[-] ARP: The requested operation requires elevation.\n";
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}
	else if (status == ERROR_OBJECT_ALREADY_EXISTS)
	{
		status = SetIpNetEntry(&arp_entry);
		logger << L"[-] Object already exists\n";
		logger << L"[+] Try change already object...\n";
		print_line(logger.str());
		logger.str(L"");
		if (status == ERROR_SUCCESS)
		{
			logger << L"[+] Set arp entry successfully!\n";
		}
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}
	else if (status == ERROR_SUCCESS)
	{
		logger << L"[+] Add arp entry successfully!\n";
		print_line(logger.str());
		logger.str(L"");
	}
	else
	{
		logger << L"[-] Couldn't add " << ip_addr << L" " << interface_addr << L" " << physic_addr << L" error:" << std::to_wstring(status) << std::endl;
		print_line(logger.str());
		logger.str(L"");
		goto FINAL;
	}

FINAL:
	WSACleanup();
	print_line(L"[*] Final Edit Arp Entry\n");
}
