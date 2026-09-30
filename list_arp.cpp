#include "list_arp.h"


// get arp table
std::unique_ptr<BYTE[]> get_ip_net_table()
{
	DWORD status{};
	DWORD size_pointer{};

	PMIB_IPNETTABLE arp_table = NULL;

	status = GetIpNetTable(NULL, &size_pointer, TRUE);
	if (status == NO_ERROR)
	{
		return nullptr;
	}
	else if (status == ERROR_INSUFFICIENT_BUFFER)
	{
		std::unique_ptr<BYTE[]> buffer = std::make_unique<BYTE[]>(size_pointer);
		arp_table = reinterpret_cast<PMIB_IPNETTABLE>(buffer.get());

		status = GetIpNetTable(arp_table, &size_pointer, TRUE);
		if (status == ERROR_SUCCESS)
		{
			return buffer;
		}
	}

	return nullptr;
}

// get all interface address
std::unique_ptr<BYTE[]> get_ip_addr_table()
{
	PMIB_IPADDRTABLE addr_table = NULL;
	DWORD size_pointer{};

	DWORD status = GetIpAddrTable(NULL, &size_pointer, TRUE);
	if (status == NO_ERROR)
	{
		return std::unique_ptr<BYTE[]>();
	}
	else if (status == ERROR_INSUFFICIENT_BUFFER)
	{
		std::unique_ptr<BYTE[]> tmp_ptr = std::make_unique<BYTE[]>(size_pointer);
		addr_table = reinterpret_cast<PMIB_IPADDRTABLE>(tmp_ptr.get());
		status = GetIpAddrTable(addr_table, &size_pointer, TRUE);
		if (status == ERROR_SUCCESS)
		{
			return tmp_ptr;
		}
	}
	return std::unique_ptr<BYTE[]>();
}

void print_ip_net_table()
{
	print_line(L"[*] ------------ Ip Net Table ------------\n");
	DWORD pre_iterface_idx = DWORD(-1);
	std::unique_ptr<BYTE[]> buffer_ip_addr;
	std::unique_ptr<BYTE[]> buffer_ip_net;
	std::wstringstream logger;

	WSADATA wsaData;
	int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsa_result != 0)
	{
		print_line(L"[-] Winsock initialized failed\n");
		return;
	}

	// get IP Address Table for mapping interface index number to ip address
	buffer_ip_addr = get_ip_addr_table();
	PMIB_IPADDRTABLE ip_addr_table = reinterpret_cast<PMIB_IPADDRTABLE>(buffer_ip_addr.get());
	PMIB_IPNETTABLE ip_net_table = NULL;

	if (ip_addr_table == NULL)
	{
		print_line(L"[-] Empty Interface retrieve\n");
		goto FINAL;
	}

	//Get interface table
	buffer_ip_net = get_ip_net_table();
	ip_net_table = reinterpret_cast<PMIB_IPNETTABLE>(buffer_ip_net.get());
	if (ip_net_table == NULL)
	{
		print_line(L"[-] Empty arp table retrieve\n");
		goto FINAL;
	}

	for (size_t i = 0; i < ip_net_table->dwNumEntries; i++)
	{
		if (ip_net_table->table[i].dwIndex != pre_iterface_idx)
		{
			pre_iterface_idx = ip_net_table->table[i].dwIndex;
			std::wstring tmp = interface_idx_to_ip(ip_addr_table, pre_iterface_idx);
			tmp = L"\nInterface: " + tmp + L":\t" + std::to_wstring(pre_iterface_idx) + L"\n";
			print_line(tmp);

			logger << std::left << L"\t"
				<< std::setw(30) << L"Internet Address"
				<< std::setw(30) << L"Physical Address"
				<< std::setw(20) << L"Type" << L"\n";
			print_line(logger.str());
			logger.str(L"");
		}
		//get ip address
		in_addr addr;
		addr.S_un.S_addr = ip_net_table->table[i].dwAddr;
		std::wstring ip_addr = covert_ipv4_to_wstring(addr);
		//get mac address
		std::wstring phys_addr = convert_phys_addr_to_wstring(ip_net_table->table[i].bPhysAddr, ip_net_table->table[i].dwPhysAddrLen);
		// get type
		std::wstring type;
		switch (ip_net_table->table[i].Type)
		{
			case MIB_IPNET_TYPE_STATIC:
			{
				type = L"Static";
				break;
			}
			case MIB_IPNET_TYPE_DYNAMIC:
			{
				type = L"Dynamic";
				break;
			}
			case MIB_IPNET_TYPE_INVALID:
			{
				type = L"Invalid";
				break;
			}
			case MIB_IPNET_TYPE_OTHER:
			{
				type = L"Other";
				break;
			}

		}

		logger << std::left << L"\t"
			<< std::setw(30) << ip_addr
			<< std::setw(30) << phys_addr
			<< std::setw(20) << type << L"\n";
		print_line(logger.str());
		logger.str(L"");
	}

FINAL:
	WSACleanup();
	print_line(L"[+]------------ Final Retrieved IP Network Table ------------\n");
}

std::wstring interface_idx_to_ip(PMIB_IPADDRTABLE ip_addr_table, DWORD index)
{
	in_addr tmp_addr;
	if (ip_addr_table == NULL)
	{
		return std::wstring();
	}

	for (size_t i = 0; i < ip_addr_table->dwNumEntries; i++)
	{
		if (index == ip_addr_table->table[i].dwIndex)
		{
			tmp_addr.s_addr = ip_addr_table->table[i].dwAddr;
			std::vector<WCHAR> buffer(INET_ADDRSTRLEN);
			if (InetNtopW(AF_INET, &tmp_addr, buffer.data(), INET_ADDRSTRLEN))
			{
				return std::wstring(buffer.data());
			}
			else
			{
				return L"INVALID IP";
			}
		}
	}
	return L"";
}
