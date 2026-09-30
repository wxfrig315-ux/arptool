# arptool — ARP Cache Tool

A Windows console tool that displays and manages the ARP cache of the local
machine.

The ARP (Address Resolution Protocol) cache maps IP addresses to MAC (physical)
addresses for hosts on the local network segment. Windows keeps this mapping in
memory to avoid repeated ARP requests and exposes it through the IP Helper API
(`Iphlpapi.dll`) and the `MIB_IPNET*` structures.

`arptool` provides three modes:

- **list** — prints every ARP cache entry using the native `GetIpNetTable` API,
  grouped by network interface, and resolves each interface index to an IP
  address with `GetIpAddrTable`.
- **add** — creates a new static ARP entry with `CreateIpNetEntry`, or falls
  back to `SetIpNetEntry` to update the record when the entry already exists.
- **delete** — removes a single ARP cache entry with `DeleteIpNetEntry`.

All interaction is done through `std::cin` / `std::cout`. There are no command
line arguments, no flags, and no configuration files.

---

## Features

- Print the full ARP cache table, including the interface IP, the entry IP
  address, its physical (MAC) address, and the entry type (Static / Dynamic /
  Other / Invalid).
- Add static ARP entries; automatically switch to an update
  (`SetIpNetEntry`) when the entry already exists.
- Delete ARP entries by IP address, optionally pinned to a specific interface.
- Automatic interface selection: when no `-if <interface_ip>` is supplied, the
  first non-loopback interface is used.
- Clean, single-binary console application.

---

## Requirements

- Windows 7 or later.
- A C++17-capable toolchain:
  - **MSVC** — Visual Studio 2019/2022 (MSVC toolset v142/v143), or
  - **MinGW-w64** (GCC), or
  - **CMake** 3.15+ (to drive either of the above).
- Windows SDK with the Windows headers and import libraries
  (`ws2_32.lib`, `iphlpapi.lib`, `shell32.lib`).

---

## Build instructions

### Option A — CMake (MSVC)

```powershell
# From the project directory
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable is `build\Release\arptool.exe`.

### Option B — CMake (MinGW-w64 + Ninja)

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Option C — MinGW-w64 (g++) directly

```powershell
g++ -std=c++17 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -O2 -o arptool.exe `
  main.cpp list_arp.cpp add_arp.cpp delete_arp.cpp utils.cpp `
  -lws2_32 -liphlpapi -lshell32
```

---

## Usage

Run the program from a console window:

```text
arptool.exe
```

```text
==============================================
          arptool - ARP Cache Tool
==============================================
Select mode:
  1) list    - print the ARP cache table of this machine
  2) add     - add or modify an ARP entry
  3) delete  - delete an ARP entry
  0) exit
> 1
```

### 1. list — print the ARP cache table

```text
[*] ------------ Ip Net Table ------------

Interface: 192.168.112.128:	15
	Internet Address              Physical Address              Type
	192.168.112.2                 00-50-56-F3-B2-5B             Dynamic
	192.168.112.254               00-50-56-F5-63-15             Dynamic
	192.168.112.255               FF-FF-FF-FF-FF-FF             Static
	224.0.0.22                    01-00-5E-00-00-16             Static
	224.0.0.251                   01-00-5E-00-00-FB             Static
	224.0.0.252                   01-00-5E-00-00-FC             Static
	239.255.255.250               01-00-5E-7F-FF-FA             Static
	255.255.255.255               FF-FF-FF-FF-FF-FF             Static
[+]------------ Final Retrieved IP Network Table ------------
```

### 2. add — add or modify an ARP entry

```text
> 2
Enter arguments (-ip <ip_addr> -mac <phys_addr> [-if <interface_ip>]):
> -ip 1.2.3.4 -mac AA-BB-CC-DD-EE-FF -if 192.168.112.128
```

Field notes:

- **ip** — required. IP address of the ARP entry to create.
- **mac** — required. Physical address in `AA-BB-CC-DD-EE-FF` form. The entry
  is created with type `MIB_IPNET_TYPE_STATIC`.
- **if** — optional. IP address of the interface the entry belongs to. When
  omitted, the first non-loopback interface found in the IP address table is
  used.
- If the entry already exists (`ERROR_OBJECT_ALREADY_EXISTS`), the tool
  automatically calls `SetIpNetEntry` and updates the existing record.

### 3. delete — delete an ARP entry

```text
> 3
Enter arguments (-ip <ip_addr> [-if <interface_ip>]):
> -ip 1.2.3.4 -if 192.168.112.128
```

Field notes:

- **ip** — required. IP address of the entry to remove.
- **if** — optional. Interface IP; when omitted, the interface index is
  resolved from the ARP cache itself, so the entry must currently exist in the
  cache.

---

## Permissions and notes

- **list** requires only standard user rights; `GetIpNetTable` is available to
  all users.
- **add** and **delete** modify global ARP state and require an **elevated**
  (Administrator) console. When run unelevated the tool reports
  `ERROR_ACCESS_DENIED` — "The requested operation requires elevation."
- The tool operates on the **local machine only**. It does not perform any
  network/remote operations.
- Static entries added with `add` live in the cache until they are deleted or
  the machine reboots; Windows does not persist them across reboots.
- **Responsible use only.** Modifying the ARP cache of a machine can disrupt
  its network connectivity and can be abused for ARP spoofing. Only run this
  tool against systems you own or are explicitly authorized to test.
- Input is read through `std::cin` and output is written through `std::cout`
  in wide (UTF-16) console mode.

---

## Project layout

- `main.cpp` — interactive entry point (`std::cin` / `std::cout`).
- `list_arp.cpp` / `list_arp.h` — ARP cache and interface table retrieval
  (`GetIpNetTable`, `GetIpAddrTable`) and the table printer.
- `add_arp.cpp` / `add_arp.h` — add or update ARP entries
  (`CreateIpNetEntry`, `SetIpNetEntry`).
- `delete_arp.cpp` / `delete_arp.h` — delete ARP entries (`DeleteIpNetEntry`).
- `utils.*` — argument parsing, IPv4/MAC conversion helpers, and console
  output helpers.
- `CMakeLists.txt` — CMake build definition.
