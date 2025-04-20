class Snaplook {
private:
    bool inSnaplook = false;

public:
    int getPerspectiveOption(void* t);
    void onMouseLocked();
    void onKeyboard(int keyCode, int action);
};