#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "snaplook.h"
#include "conf.h"
#include "imgui.h"
#include <cstddef>
#include <span>
#include <libhat.hpp>
#include <link.h>

Snaplook snapLook;

extern "C" void __attribute__((visibility("default"))) mod_preinit() {
}

extern "C" __attribute__((visibility("default"))) void mod_init() {
    Conf::load();
    initImgui();

    auto gw = dlopen("libmcpelauncher_gamewindow.so", 0);
    game_window_is_mouse_locked = (decltype(game_window_is_mouse_locked))dlsym(gw, "game_window_is_mouse_locked");
    game_window_get_primary_window = (decltype(game_window_get_primary_window))dlsym(gw, "game_window_get_primary_window");
    game_window_add_window_creation_callback = (decltype(game_window_add_window_creation_callback))dlsym(gw, "game_window_add_window_creation_callback");
    game_window_add_keyboard_callback = (decltype(game_window_add_keyboard_callback))dlsym(gw, "game_window_add_keyboard_callback");

    game_window_add_window_creation_callback(NULL, [](void* user) {
        game_window_add_keyboard_callback(game_window_get_primary_window(), NULL, [](void* user, int keyCode, int action) -> bool {
            if(Conf::changingKey) {
                if(action == 0) {
                    Conf::snaplookKey = keyCode;
                    showKeyWindow();
                }
                return true;
            }
            snapLook.onKeyboard(keyCode, action);
            return false;
        });
    });

    auto mcLib = dlopen("libminecraftpe.so", 0);

    std::span<std::byte> range1, range2;

    auto callback = [&](const dl_phdr_info& info) {
        if(auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };

    dl_iterate_phdr(
        [](dl_phdr_info* info, size_t, void* data) {
            return (*static_cast<decltype(callback)*>(data))(*info);
        },
        &callback);

    auto VanillaCameraAPI_typeinfo_name = hat::find_pattern(range1, hat::object_to_signature("16VanillaCameraAPI")).get();
    auto VanillaCameraAPI_typeinfo = hat::find_pattern(range2, hat::object_to_signature(VanillaCameraAPI_typeinfo_name)).get() - sizeof(void*);
    auto VanillaCameraAPI_vtable = hat::find_pattern(range2, hat::object_to_signature(VanillaCameraAPI_typeinfo)).get() + sizeof(void*);
    auto VanillaCameraAPI_getPlayerViewPerspectiveOption = reinterpret_cast<int (**)(void*)>(VanillaCameraAPI_vtable) + 6;  // Change to 7 when 1.21.80 releases

    VanillaCameraAPI_getPlayerViewPerspectiveOption_orig = *VanillaCameraAPI_getPlayerViewPerspectiveOption;

    *VanillaCameraAPI_getPlayerViewPerspectiveOption = [](void* t) -> int {
        return snapLook.getPerspectiveOption(t);
    };
}
