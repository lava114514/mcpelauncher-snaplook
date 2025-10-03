#include "snaplook.h"
#include "main.h"
#include "conf.h"

int Snaplook::getPerspectiveOption(void* t) {
    if(inSnaplook) {
        return 1;  // third person back
    }
    return VanillaCameraAPI_getPlayerViewPerspectiveOption_orig(t);
}

void Snaplook::onKeyboard(int keyCode, int action) {
    if(keyCode == Conf::snaplookKey) {
        switch(action) {
        case 0:
            if(!game_window_is_mouse_locked(game_window_get_primary_window())) {
                return;
            }
            inSnaplook = true;
            break;
        case 2:
            if(inSnaplook) {
                inSnaplook = false;
            }
        default:
            break;
        }
    }
}
