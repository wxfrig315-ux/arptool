# PLAYBOOK — Chuyển module DLL thành console EXE và đưa lên GitHub

> File mô tả toàn bộ quy trình đã thực thi trên project `arptool` (thư mục `arp`),
> dùng lại được cho các project module tương tự (thư mục chứa `feature_params.h`,
> `dllmain.cpp`, `export.def`).

---

## TỔNG QUAN QUY TRÌNH

1. **Refactor** module DLL → console EXE (bỏ `FeatureParams`, thêm `main.cpp`).
2. **Thêm file build** (`CMakeLists.txt`).
3. **Viết lại README** tiếng Anh theo template `lspipe`.
4. **Tạo repo GitHub + push** bằng tài khoản đã đăng nhập trên máy (Git Credential Manager).

---

## BƯỚC 1 — Refactor module DLL thành console EXE

### 1.1. Xoá các file thuộc DLL/module

```powershell
Remove-Item feature_params.h, dllmain.cpp, export.def
```

### 1.2. Đổi signature các hàm feature

Quy tắc chuyển đổi từ `FeatureParams`:

| Trước (module DLL)                     | Sau (console EXE)                          |
|:-------------------------------------- |:------------------------------------------ |
| `void func(FeatureParams* p)`          | `void func(const std::wstring& args)`      |
| `p->StrInput`                          | tham số `args` truyền vào hàm              |
| `p->Output1(p->TaskId, msg, vec)`      | `print_line(msg)` (ghi ra `std::wcout`)    |
| `p->Final1(p->TaskId, code, msg, vec)` | `print_line(msg)` cuối hàm                 |
| hàm phụ nhận `FeatureParams*` nhưng không dùng | bỏ tham số (`get_ip_net_table()`)   |

Thêm helper in console vào `utils.h` / `utils.cpp`:

```cpp
// utils.h
void print_line(const std::wstring& msg);

// utils.cpp
void print_line(const std::wstring& msg)
{
    std::wcout << msg << std::endl;
}
```

Đổi `parse_args(FeatureParams* p)` thành `parse_args(const std::wstring& input)`
(vẫn dùng `CommandLineToArgvW(input.c_str(), &nArgs)`).

### 1.3. Sửa header

- Thêm `#pragma once` cho mọi header còn thiếu.
- Bỏ `#include "feature_params.h"` trong `utils.h`.
- Header tự include chính nó (ví dụ `list_arp.h` include `list_arp.h`) → bỏ dòng thừa.
- Include `<shellapi.h>` ở file dùng `CommandLineToArgvW`.

### 1.4. Tạo `main.cpp` — entry point menu (theo style lspipe)

```cpp
#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>

#include "list_arp.h"    // các header feature của project
#include "add_arp.h"
#include "delete_arp.h"

static void print_banner()
{
    std::wcout << L"==============================================\n";
    std::wcout << L"          arptool - ARP Cache Tool\n";
    std::wcout << L"==============================================\n";
    std::wcout << L"Select mode:\n";
    std::wcout << L"  1) list    - ...\n";
    std::wcout << L"  2) add     - ...\n";
    std::wcout << L"  3) delete  - ...\n";
    std::wcout << L"  0) exit\n";
    std::wcout << L"> ";
    std::wcout.flush();
}

int main()
{
    // Bật chế độ console wide (UTF-16) để in dấu tiếng Việt/Unicode đúng.
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    for (;;)
    {
        print_banner();
        std::wstring line;
        if (!std::getline(std::wcin, line)) break;

        if      (line == L"1") print_ip_net_table();
        else if (line == L"2") { /* đọc args -> set_ip_net_entry(args); */ }
        else if (line == L"3") { /* đọc args -> delete_ip_net_entry(args); */ }
        else if (line == L"0" || line == L"q" || line == L"quit" || line == L"exit") break;
        else if (!line.empty()) std::wcout << L"[-] Unknown option: " << line << L"\n";
    }
    return 0;
}
```

Lưu ý:
- Chỉ dùng **wide I/O** (`std::wcout`, `std::wcin`) sau khi `_setmode(_O_U16TEXT)`.
  Trộn narrow/wide sẽ gây crash assert.
- Không có command line argument, không flag, không config file — mọi tương tác
  qua `std::cin` / `std::cout` (đúng template lspipe).

### 1.5. Bug thường gặp cần fix khi refactor

- `if (x = -1)` → gán thay vì so sánh → sửa thành `if (x == DWORD(-1))`.
- `if (result) WSACleanup();` sau `WSAStartup` → logic ngược; sửa thành
  chỉ cleanup khi `WSAStartup` thành công (`result == 0`).
- Hàm trả `std::unique_ptr<BYTE[]>` có nhánh rơi ra ngoài không return → thêm
  `return nullptr;` cuối hàm.
- `logger << "narrow string"` trong `wstringstream` → đổi hết thành `L"..."`.
- Header cột bảng in nhầm (`tType`) → sửa lại đúng (`Type`).

---

## BƯỚC 2 — Thêm `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.15)
project(arptool CXX)   # đổi tên project cho phù hợp

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_executable(arptool          # đổi tên exe
    main.cpp
    list_arp.cpp                # liệt kê các .cpp của project
    add_arp.cpp
    delete_arp.cpp
    utils.cpp
)

target_link_libraries(arptool PRIVATE ws2_32 iphlpapi shell32)  # libs theo project
```

Build kiểm chứng (khi cần):

```powershell
cmake -S . -B build
cmake --build build --config Release
```

---

## BƯỚC 3 — Viết lại README tiếng Anh theo template lspipe

Cấu trúc bắt buộc của template:

```
# <tên> — <mô tả ngắn>

<2 đoạn giới thiệu: khái niệm nền + cơ chế bên dưới>
<tên-tool> provides N modes:
- **mode1** — ...
- **mode2** — ...

All interaction is done through `std::cin` / `std::cout`. There are no command
line arguments, no flags, and no configuration files.

---

## Features          (bullet list tính năng)
## Requirements      (OS, toolchain C++17, SDK/libs)
## Build instructions
   ### Option A — CMake (MSVC)
   ### Option B — CMake (MinGW-w64 + Ninja)
   ### Option C — MinGW-w64 (g++) directly
## Usage             (transcript menu + từng mode + "Field notes" giải thích tham số)
## Permissions and notes   (quyền cần thiết, lưu ý, "Responsible use only.")
## Project layout    (mô tả từng file)
```

Gợi ý lệnh build g++ trực tiếp (thay tên file/libs):

```powershell
g++ -std=c++17 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -O2 -o <tool>.exe `
  main.cpp <các file>.cpp -lws2_32 -liphlpapi -lshell32
```

---

## BƯỚC 4 — Tạo repo GitHub và push (dùng tài khoản đã lưu trên máy)

### 4.1. Kiểm tra credential đã lưu

```powershell
cmdkey /list | Select-String "git:https://github.com"
```

Lấy username + token từ Git Credential Manager (KHÔNG hardcode token vào file):

```powershell
"protocol=https`nhost=github.com`n" | git credential fill
```

→ Trả về `username=...` và `password=<token gho_...>`.

### 4.2. Init repo local + commit

```powershell
git init -b main
git add -A
git -c user.name="<username>" -c user.email="<username>@users.noreply.github.com" `
    commit -m "<mô tả commit>"
```

### 4.3. Tạo repo trên GitHub bằng REST API

Viết JSON body vào file tạm (tránh lỗi quote JSON trong PowerShell):

```powershell
# repo.json
{"name":"<repo-name>","description":"<mô tả>","private":true}
```

```powershell
curl.exe -s -X POST `
  -H "Authorization: Bearer <token>" `
  -H "Accept: application/vnd.github+json" `
  -d "@repo.json" https://api.github.com/user/repos
```

Kiểm tra: `full_name`, `html_url`, `private`.

### 4.4. Thêm remote và push

```powershell
git remote add origin https://github.com/<username>/<repo-name>.git
git push -u origin main
```

(Git Credential Manager tự cung cấp credential khi push — không cần dán token.)

### 4.5. Đổi private ↔ public

```powershell
# visibility.json: {"private": false}  hoặc  {"private": true}
curl.exe -s -X PATCH `
  -H "Authorization: Bearer <token>" `
  -H "Accept: application/vnd.github+json" `
  -d "@visibility.json" https://api.github.com/repos/<username>/<repo-name>
```

---

## CHECKLIST NHANH

- [ ] Xoá `feature_params.h`, `dllmain.cpp`, `export.def`
- [ ] Bỏ `FeatureParams` khỏi mọi signature; output → `print_line`/`std::wcout`
- [ ] Sửa header (`#pragma once`, include thừa/tự-include)
- [ ] Tạo `main.cpp` menu (`_setmode(_O_U16TEXT)` + `std::wcin`/`std::wcout`)
- [ ] Fix bug: `=` vs `==`, `WSACleanup`, return thiếu, narrow/wide string
- [ ] Thêm `CMakeLists.txt` (exe + link libs)
- [ ] README tiếng Anh theo template lspipe (Features/Requirements/Build/Usage/Permissions/Project layout)
- [ ] `git init -b main` + commit
- [ ] `git credential fill` → username + token
- [ ] `POST /user/repos` (JSON qua file tạm) → tạo repo
- [ ] `git remote add origin` + `git push -u origin main`
- [ ] `PATCH /repos/...` đổi visibility nếu cần
- [ ] Xoá file JSON tạm chứa token sau khi dùng
