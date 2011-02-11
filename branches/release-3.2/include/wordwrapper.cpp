TWordWrapper::TWordWrapper(TEXTWIDTHFUNCTION twf) {
    setTextWidthFunction(twf);
    wrapped = false;
}

void TWordWrapper::setTextWidthFunction(TEXTWIDTHFUNCTION twf) {
    textWidthFunction = twf;
}

int TWordWrapper::wrap(char*text, int width){
    return wrap(text, width, 0x7ffffff);
}

int TWordWrapper::wrap(char*text, int width, int maxLines){
  char*t; //,buf;
  if(width < 20)return 0;
  if(text[0]==0)return 0;
  linebreak.DeleteAll(false);
  linebreak.Add(0);
  int last_linebreak=0;
  int last_length=0;
  int last_break_char=0;
  int length;
  int cn,cut_cn;
  int text_length=strlen(text);
  wrapped = true;
  for(cn=0;cn<=text_length;cn++){
    char c=text[cn];
    if(c==' ' || c=='-' || c==0 || c=='\n'){ //breaking character
      if(c==' ' || c==0){
        cut_cn=cn;
      }else if(c=='\n'){
        if(text[cn-1]=='\r'){
          cut_cn=cn-1;
        }else{
          cut_cn=cn;
        }
      }else{ //visible breaking character
        cut_cn=cn+1;
      }
      t=text+last_linebreak;
      if(cut_cn>last_linebreak){
        length=textWidthFunction(t,cut_cn-last_linebreak);
      }else{
        length=0;
      }
      if (length > width) { //wrap
        if(last_length==0){ //nowhere to break the line!
          do{
            cut_cn--;
            if(cut_cn<=last_linebreak){
              numLines=0;
              return 0; //something's gone badly wrong!
            }else if(cut_cn==last_linebreak+1){
              break;
            }
            length = textWidthFunction(t,cut_cn-last_linebreak);
          } while( length > width );
          linebreak.Add(cut_cn);
          last_linebreak=cut_cn;
          cn=cut_cn;last_length=0;last_break_char=cn;
          if (--maxLines < 0) {
            numLines=linebreak.NumItems-1;
            return numLines;
          }
        }else{ //there is a breaking character in the line
          cn=last_break_char+1;
          while(text[cn]==' ')cn++;
          linebreak.Add(cn); //leave space or - on end of previous line
          last_linebreak=cn;
          last_length=0;last_break_char=last_linebreak;
          if (--maxLines < 0) {
            numLines=linebreak.NumItems-1;
            return numLines;
          }
        }
      }else{ //don't need to wrap yet
        if(c=='\n'){   //return
          linebreak.Add(cn+1);
          last_linebreak=cn+1;
          last_length=0;last_break_char=last_linebreak;
          if(--maxLines<0){
            numLines=linebreak.NumItems-1;
            return numLines;
          }
        }else{
          last_length=length;
          last_break_char=cn;
        }
      }
    }
    if(text[cn]==0){ //reached end
      break;
    }
  } //next cn

  numLines=linebreak.NumItems;
  linebreak.Add(cn);
  return numLines;
}

int TWordWrapper::getLineFromCharacterIndex(int ci) {
  if(wrapped){
    int n;
    for(n=0;n<numLines;n++){
      if (linebreak[n] > ci) {
        return n-1;
      }
    }
    return numLines-1;
  }else{
    return 0;
  }
}

EasyStr TWordWrapper::getHardWrappedString(char*text, int width) {
    wrap(text,width);
    EasyStr ret;
    int text_length = strlen(text);
    int linebreak_index = 1;
    char new_character[3]; new_character[1] = 0; new_character[2] = 0;
    for (int cp = 0; cp < text_length; cp++) {
        if (linebreak_index < numLines) while (linebreak[linebreak_index] <= cp){
            new_character[0] = '\r';
            new_character[1] = '\n';
            linebreak_index++;

            ret += new_character;
            new_character[1] = 0;
            if (linebreak_index >= numLines) break;

        }
        new_character[0] = text[cp];
        ret += new_character;
    }

    return ret;
}
