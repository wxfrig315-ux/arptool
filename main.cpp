#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>

#include "list_arp.h"
#include "add_arp.h"
#include "delete_arp.h"

static void print_banner()
{
    std::wcout << L"==============================================\n";
    std::wcout << L"          arptool - ARP Cache Tool\n";
    std::wcout << L"==============================================\n";
    std::wcout << L"Select mode:\n";
    std::wcout << L"  1) list    - print the ARP cache table of this machine\n";
    std::wcout << L"  2) add     - add or modify an ARP entry\n";
    std::wcout << L"  3) delete  - delete an ARP entry\n";
    std::wcout << L"  0) exit\n";
    std::wcout << L"> ";
    std::wcout.flush();
}

int main()
{
    // Enable wide (UTF-16) console mode so all messages print correctly.
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    for (;;)
    {
        print_banner();

        std::wstring line;
        if (!std::getline(std::wcin, line))
        {
            break;
        }

        if (line == L"1")
        {
            print_ip_net_table();
        }
        else if (line == L"2")
        {
            std::wcout << L"Enter arguments (-ip <ip_addr> -mac <phys_addr> [-if <interface_ip>]):\n> ";
            std::wcout.flush();
            if (std::getline(std::wcin, line) && !line.empty())
            {
                set_ip_net_entry(line);
            }
            else
            {
                std::wcout << L"[-] No arguments given, back to menu...\n";
            }
        }
        else if (line == L"3")
        {
            std::wcout << L"Enter arguments (-ip <ip_addr> [-if <interface_ip>]):\n> ";
            std::wcout.flush();
            if (std::getline(std::wcin, line) && !line.empty())
            {
                delete_ip_net_entry(line);
            }
            else
            {
                std::wcout << L"[-] No arguments given, back to menu...\n";
            }
        }
        else if (line == L"0" || line == L"q" || line == L"quit" || line == L"exit")
        {
            break;
        }
        else if (!line.empty())
        {
            std::wcout << L"[-] Unknown option: " << line << L"\n";
        }
    }

    return 0;
}
