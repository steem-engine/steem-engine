TScreenSaver* TScreenSaver::instance = NULL;
volatile bool TScreenSaver::screensaver_active = false;
bool TScreenSaver::screen_saver_turned_on = false;
DWORD TScreenSaver::screen_saver_activate_time;
bool TScreenSaver::postponed = false;
int TScreenSaver::screen_width = 640, TScreenSaver::screen_height = 480;
const int TScreenSaver::BUFFER_WIDTH = 5*32+30, TScreenSaver::BUFFER_HEIGHT = 200;
const int TScreenSaver::ANIM_LENGTH = 64*5+14, TScreenSaver::COLOUR_SPEED = 16;

#ifdef WIN32

const char* TScreenSaver::WINDOW_CLASS_NAME = "Steem Screen Saver";
bool TScreenSaver::timer_running = false;
DWORD TScreenSaver::timer_id = 0;
int TScreenSaver::screen_saver_timeout_seconds = 600;
LONG TScreenSaver::mousePos = 0;

void TScreenSaver::prepareTimer(){
    if (isActive()) {   //use timer to animate screensaver
        timer_id = SetTimer(NULL, timer_id, 30, TScreenSaver::animationTimerProc);
    }else {
        SystemParametersInfo(SPI_GETSCREENSAVEACTIVE,0,&screen_saver_turned_on,0);
        log_DELETE_SOON(EasyStr("Screen saver turned on? ")+screen_saver_turned_on);
        if (screen_saver_turned_on){
            SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT,0,&screen_saver_timeout_seconds,0);
            screen_saver_activate_time = GetTickCount() + screen_saver_timeout_seconds*1000;
            timer_id = SetTimer(NULL, timer_id, screen_saver_timeout_seconds*1000, TScreenSaver::activationTimerProc);
            timer_running = true;
            postponed = false;
            log_DELETE_SOON(EasyStr("Screen saver will activate in ")+screen_saver_timeout_seconds+"s, at "+screen_saver_activate_time);
            updateTimer();
        }
    }
}

void TScreenSaver::updateTimer(){
    DWORD current_time = GetTickCount();
    if (screen_saver_activate_time < current_time) screen_saver_activate_time = current_time + 100;
    log_DELETE_SOON(EasyStr("Screen saver set to activate in ")+(screen_saver_activate_time - current_time)+"ms");
    timer_id = SetTimer(NULL, timer_id, screen_saver_activate_time - current_time, TScreenSaver::activationTimerProc);
    timer_running = true;
}

void TScreenSaver::killTimer(){
    timer_running = false;
    KillTimer(NULL, timer_id);
    timer_id = 0;
}

void TScreenSaver::postpone(){
    if (timer_running){
        screen_saver_activate_time = GetTickCount() + screen_saver_timeout_seconds*1000;
        postponed = true;
    }
}

void TScreenSaver::activate() {
    log_DELETE_SOON( "Scrrensaver active!!!" );
    killTimer();
    if (instance != NULL) {
        hide();
    }
    instance = new TScreenSaver();
    instance->animation_counter = 0;
    screensaver_active = true;
    ShowCursor(false);
    prepareTimer();

}

void TScreenSaver::hide() {
    if (instance != NULL) {
        delete instance->word_wrapper;
        instance->deleteBuffer();
        DestroyWindow(instance->handle);
        UnregisterClass(WINDOW_CLASS_NAME, Inst);
        delete instance;
        instance = NULL;
        screensaver_active = false;
        prepareTimer();
        ShowCursor(true);
    }
}


VOID CALLBACK TScreenSaver::activationTimerProc(HWND, UINT, UINT, DWORD) {
    if (FullScreen) {
        if (postponed){
            updateTimer();
            postponed = false;
        } else {
            activate();
        }
    } else {
        killTimer();
    }
}

VOID CALLBACK TScreenSaver::animationTimerProc(HWND, UINT, UINT, DWORD) {
    if (FullScreen) {
        if (isActive()) {  //animate screen saver
            instance->animate();
        }
    } else {
        killTimer();
    }
}

void TScreenSaver::animate() {
    animation_counter++;
    if (animation_counter > ANIM_LENGTH ){
        initAnimation();
    }
    int frame = animation_counter & 63;
    if (frame == 14) {
        advanceColour();
    }
    ZeroMemory(buffer_memory, 32*buffer_line_length);
    int col = colour_convert(anim_col_next[0], anim_col_next[1], anim_col_next[2]);
    for (int c = 0; c < 5; c++) {
    // -12 -9 -6 -3 -0
        frame = (animation_counter - (3 * c));
        if (frame >= 0 && frame < ANIM_LENGTH-14) {
            frame &= 63;
            if (frame > (64-14)) {
                frame = 64 - frame;
            } else if (frame > 14) {
                frame = 14;
            } else if (frame < 3) {
                col = colour_convert(anim_col[0], anim_col[1], anim_col[2]);
            }
            draw_char_routine(osd_font+(frame*64),buffer_memory,c * 32, 0, buffer_line_length, col, 32);
        }
    }
    for (int c = 0; c <= OSD_LOGO_W / 32; c++) {
        draw_char_routine(osd_font+((50+c)*64),buffer_memory,(5 * 16 - OSD_LOGO_W/2) + c * 32, 12 - OSD_LOGO_H/2, buffer_line_length, 0xffffff, 32);
    }
    SetBitmapBits( buffer_bitmap, 32 * buffer_line_length, buffer_memory);
    if (animation_counter == 20) {
        initMessage();
        RECT text_rect;
        text_rect.left = 0; text_rect.right = BUFFER_WIDTH;
        text_rect.top = 32; text_rect.bottom = BUFFER_HEIGHT;
        SetTextColor(buffer_dc, 0xffffff);
        SetBkColor(buffer_dc, 0);
        DrawText(buffer_dc, message_text.Text, strlen(message_text.Text),
                &text_rect, DT_TOP | DT_LEFT | DT_NOPREFIX);
    }
    SendMessage(handle, WM_PAINT, 0, 0);

}

void TScreenSaver::advanceColour() {
    if (animation_counter > ANIM_LENGTH - 14){
        for (int ci = 0; ci < 3; ci++) {
            anim_col[ci] = anim_col_next[ci];
            anim_col_next[ci] = 0;
        }
    } else {
        for (int ci = 0; ci < 3; ci++) {
            anim_col[ci] = anim_col_next[ci];
            int dir = anim_col_target[ci] - anim_col[ci];
            if (dir < -COLOUR_SPEED) {
                anim_col_next[ci] -= COLOUR_SPEED;
            } else if (dir > COLOUR_SPEED) {
                anim_col_next[ci] += COLOUR_SPEED;
            } else {
                anim_col_next[ci] = anim_col_target[ci];
                anim_col_target[ci] = random(256);
            }
        }
    }
}

void TScreenSaver::initAnimation() {
    animation_counter = 0;
    for (int ci = 0; ci < 3; ci++) {
        anim_col[ci] = 0;
        anim_col_next[ci] = random(256);
        anim_col_target[ci] = random(256);
    }
    ZeroMemory(buffer_memory, buffer_size);
    SetBitmapBits( buffer_bitmap, buffer_size, buffer_memory);

    message_x = random(screen_width - BUFFER_WIDTH);
    message_y = random(screen_height - BUFFER_HEIGHT);
}

void TScreenSaver::initMessage() {
    int scroller_number = random(osd_scroller_array.NumStrings);
    message_text = get_osd_scroller_text(scroller_number);
    message_text = word_wrapper->getHardWrappedString(message_text.Text, BUFFER_WIDTH);

}


LRESULT WINAPI TScreenSaver::WndProc(HWND handle, UINT mess, WPARAM wParam, LPARAM lParam) {
    if (mess == WM_MOUSEMOVE || mess == WM_KEYDOWN) {
//        hide();
        return 0;
    } else if (mess == WM_PAINT && isActive()) {
        instance->paint();
    }
    if (mess == WM_SYSCOMMAND) {
        switch (wParam & 0xFFF0){
        case SC_SCREENSAVE: case SC_MONITORPOWER:
            return 0;
        }
    }

    return DefWindowProc(handle, mess, wParam, lParam);
}

void TScreenSaver::paint() {
    HDC dc = GetWindowDC(handle);
    if (animation_counter == 1) {
      HBRUSH brush = (HBRUSH)(GetStockObject(BLACK_BRUSH));
      RECT rc; rc.left = 0; rc.right = screen_width; rc.top = 0; rc.bottom = screen_height;
      FillRect(dc, &rc, brush);
    }
    if (instance->buffer_valid) {
        BitBlt(dc, message_x, message_y, BUFFER_WIDTH, BUFFER_HEIGHT, instance->buffer_dc, 0, 0, SRCCOPY);
    }

    ReleaseDC(handle, dc);
}


int TScreenSaver::getTextWidth(char* text, int character_count){
    SIZE size;
    if (instance == NULL) {
        return character_count*10;
    }
    char save_char = text[character_count];
    text[character_count] = 0;
    GetTextExtentPoint32(instance->buffer_dc, text, character_count, &size);
    text[character_count] = save_char;
    return size.cx;
}

TScreenSaver::TScreenSaver() {
    WNDCLASS wc={0,WndProc,0,0,Inst,hGUIIcon[RC_ICO_APP],
                    PCArrow,(HBRUSH)(GetStockObject(BLACK_BRUSH)),NULL,WINDOW_CLASS_NAME};
    RegisterClass(&wc);

    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    word_wrapper = new TWordWrapper(getTextWidth);

    handle = CreateWindowEx(0,WINDOW_CLASS_NAME,"",0 ,
                              0,0, screen_width,
                              screen_height,
                              StemWin,NULL,Inst,NULL);
//    SetWindowLong(handle,GWL_STYLE,WS_POPUP);
//    MoveWindow(handle,0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,0);
    ShowWindow(handle,SW_SHOW);
    SetWindowPos(handle,HWND_TOPMOST,0,0,screen_width,screen_height,SWP_NOMOVE | SWP_NOSIZE);
    UpdateWindow(handle);
    SetCursor(NULL);
    buffer_bitmap = NULL;
    initBuffer();
    initAnimation();
}

bool TScreenSaver::isActive(){
    return (screensaver_active);
}

void inline TScreenSaver::checkMessage(MSG* mess){
    
    if (FullScreen){
        if (screen_saver_turned_on){
            switch(mess->message){
            case WM_MOUSEMOVE:
                if (mousePos == mess->lParam){
                    break;
                }
                mousePos = mess->lParam;
                if (isActive() && mess->time < screen_saver_activate_time + 1000) {
                    break; //show for at least one second
                }
            case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN:
            case WM_KEYDOWN:
                if (isActive()){
                    hide();
                }else{
                    postpone();
                }
            }
        }
    }
}


bool TScreenSaver::initBuffer() {
    deleteBuffer();

    HDC dc = GetDC(handle);
    buffer_bitmap = CreateCompatibleBitmap(dc, BUFFER_WIDTH, BUFFER_HEIGHT);

    if (buffer_bitmap == NULL) return false;      //failed to create buffer

    BITMAP bitmap_info;
    GetObject(buffer_bitmap, sizeof(BITMAP), &bitmap_info);
    buffer_line_length = bitmap_info.bmWidthBytes;

    buffer_size = buffer_line_length * bitmap_info.bmHeight;

    buffer_dc = CreateCompatibleDC(dc);

    ReleaseDC(handle,dc);

    SelectObject(buffer_dc, buffer_bitmap);
    SelectObject(buffer_dc, fnt);

    try{
        buffer_memory = new BYTE[buffer_size + 1];
        ZeroMemory(buffer_memory, buffer_size);
    }catch (...){
        buffer_memory = NULL;
        deleteBuffer();
        return false;
    }
    buffer_valid = true;
    draw_char_routine = jump_osd_draw_char[BytesPerPixel-1];
    return buffer_valid;
}

void TScreenSaver::deleteBuffer() {
    if (buffer_bitmap != NULL){
        DeleteDC(buffer_dc);   buffer_dc = NULL;
        DeleteObject(buffer_bitmap); buffer_bitmap = NULL;
        delete[] buffer_memory;
    }
    draw_char_routine = osd_draw_char_dont;
    buffer_valid = false;

}

#else

#include "x/x_screen_saver.cpp"

#endif
