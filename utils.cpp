#include "utils.h"

void print_line(const std::wstring& msg)
{
    std::wcout << msg << std::endl;
}

std::map<std::wstring, std::wstring> parse_args(const std::wstring& input)
{
    std::wstringstream logger;
    std::map<std::wstring, std::wstring> options;
    std::vector<std::wstring> args;

    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(input.c_str(), &nArgs);

    if (NULL == szArglist)
    {
        logger << L"CommandLineToArgvW failed\n";
        print_line(logger.str());
        return std::map<std::wstring, std::wstring>();
    }

    args.assign(szArglist, szArglist + nArgs);
    LocalFree(szArglist);

    for (size_t i = 0; i < args.size(); ++i)
    {
        std::wstring arg = args[i];
        if (arg == L"-ip")
        {
            if (i + 1 < args.size()) options[L"ip"] = args[++i];
        }
        else if (arg == L"-if")
        {
            if (i + 1 < args.size()) options[L"if"] = args[++i];
        }
        else if (arg == L"-mac")
        {
            if (i + 1 < args.size()) options[L"mac"] = args[++i];
        }
        else
        {
            logger << L"Unexpected parameter: " << arg << std::endl;
            print_line(logger.str());
            break;
        }
    }
    return options;
}


std::wstring covert_ipv4_to_wstring(in_addr input)
{
    std::wstring ip_str;
    std::vector<WCHAR> ip_buf(16);

    ip_str = InetNtopW(AF_INET, &input, (PWSTR)ip_buf.data(), ip_buf.size());

    return ip_str;
}

std::wstring convert_phys_addr_to_wstring(BYTE phys_addr[], DWORD phys_addr_len)
{
    std::vector<BYTE> phys_buf(phys_addr, phys_addr + phys_addr_len);
    std::wstring mac_str;
    if (phys_addr == NULL || phys_addr_len == 0)
    {
        return std::wstring();
    }

    for (size_t i = 0; i < phys_buf.size(); i++)
    {
        if (i == phys_addr_len - 1)
        {
            mac_str += byte_to_hex(phys_buf[i]);
        }
        else
        {
            mac_str += byte_to_hex(phys_buf[i]) + L"-";
        }
    }
    return mac_str;
}

DWORD convert_ip_addr_to_dword(std::wstring input)
{
    in_addr addr;
    if (input.empty())
    {
        return INADDR_NONE;
    }
    if (InetPtonW(AF_INET, input.c_str(), &addr) == 1)
    {
        return addr.S_un.S_addr;
    }
    else
    {
        return INADDR_NONE;
    }
}


DWORD get_interface_index_by_ip_addr(DWORD target_ip, PMIB_IPNETTABLE pTable)
{
    if (pTable == NULL || target_ip == 0)
    {
        return -1;
    }

    for (DWORD i = 0; i < pTable->dwNumEntries; ++i)
    {
        if (pTable->table[i].dwAddr == target_ip)
        {
            return pTable->table[i].dwIndex;
        }
    }
    return -1; // Not found
}

DWORD get_interface_index_by_interface_ip(std::wstring target_ip, PMIB_IPADDRTABLE table)
{
    if (target_ip.empty() || table == NULL)
    {
        return -1;
    }

    DWORD ip_addr_dword = convert_ip_addr_to_dword(target_ip);

    for (size_t i = 0; i < table->dwNumEntries; i++)
    {
        if (table->table[i].dwAddr == ip_addr_dword)
        {
            return table->table[i].dwIndex;
        }
    }

    return -1;

}

std::wstring byte_to_hex(BYTE b)
{
    std::wstringstream oss;
    oss << std::hex << std::uppercase << std::setw(2) << std::setfill(L'0') << (int)b;
    return oss.str();
}

std::vector<BYTE> wstring_to_phys_addr(std::wstring phys_addr)
{
    if (phys_addr.empty())
    {
        return std::vector<BYTE>();
    }
    std::vector<BYTE> result;
    std::wstringstream ss(phys_addr);

    std::wstring token;
    while (std::getline(ss, token, L'-'))
    {
        BYTE b = static_cast<BYTE>(std::stol(token, nullptr, 16));
        result.push_back(b);
    }

    return result;
}
