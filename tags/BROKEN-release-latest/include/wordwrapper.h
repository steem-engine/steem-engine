typedef int TEXTWIDTHFUNCTION(char*, int);
typedef TEXTWIDTHFUNCTION* LPTEXTWIDTHFUNCTION;

class TWordWrapper {
    private:
        LPTEXTWIDTHFUNCTION textWidthFunction;
        bool wrapped;
        int numLines;
        DynamicArray<int> linebreak;
    public:
        TWordWrapper(LPTEXTWIDTHFUNCTION twf);
        void setTextWidthFunction(LPTEXTWIDTHFUNCTION twf);
        int wrap(char* text, int width);
        int wrap(char* text, int width, int maxLines);
        EasyStr getHardWrappedString(char* text, int width);
        int getLineFromCharacterIndex(int ci);
        DynamicArray<int> getLineBreaks();
};


