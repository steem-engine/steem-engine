class TScreenSaver{
private:
    static TScreenSaver* instance;
    volatile static bool screensaver_active;
    static int screen_width, screen_height;
    static const int BUFFER_WIDTH, BUFFER_HEIGHT;
    static const int ANIM_LENGTH;
    static const int COLOUR_SPEED;
    static bool postponed;
    static bool screen_saver_turned_on;
    static DWORD screen_saver_activate_time;

#ifdef WIN32
    static const char* WINDOW_CLASS_NAME;
    static bool timer_running;
    static DWORD timer_id;
    static int screen_saver_timeout_seconds;
    static LONG mousePos;

    HWND handle;
    int animation_counter;
    int anim_col[3], anim_col_next[3], anim_col_target[3];
    HBITMAP buffer_bitmap;
    HDC buffer_dc;
    int buffer_line_length;
    int buffer_size;
    BYTE *buffer_memory;
    bool buffer_valid;
    LPOSDDRAWCHARPROC draw_char_routine;
    EasyStr message_text;
    TWordWrapper *word_wrapper;
    int message_x, message_y;
#endif

    bool initBuffer();
    void initAnimation();
    void initMessage();
    void deleteBuffer();
    void animate();
    void advanceColour();
    void paint();

    static void updateTimer();

public:
    static void activate();
    static void hide();

    static void prepareTimer();
    static void killTimer();
    static void postpone();

    static bool isActive();

#ifdef WIN32
    static void checkMessage(MSG* mess);
    static LRESULT WINAPI WndProc(HWND,UINT,WPARAM,LPARAM);
    static VOID CALLBACK activationTimerProc(HWND,UINT,UINT,DWORD);
    static VOID CALLBACK animationTimerProc(HWND,UINT,UINT,DWORD);
#endif

    static int getTextWidth(char*, int);

    TScreenSaver();
};

