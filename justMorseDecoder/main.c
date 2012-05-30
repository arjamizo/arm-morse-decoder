#include <string.h>
#include <stdio.h>
const char *codes[29]=
{"1434\0",
"34141414\0",
"34143414\0",
"341414\0",
"14\0",
"14143414\0",
"343414\0",
"14141414\0",
"1414\0",
"14343434\0",
"341434\0",
"14341414\0",
"3434\0",
"3414\0",
"343434\0",
"14343414\0",
"34341434\0",
"143414\0",
"141414\0",
"34\0",
"141434\0",
"14141434\0",
"143434\0",
"34141434\0",
"34143434\0",
"34341414\0",
"34\0",
"5\0",
"\0"};
//4 oznacza jedna przerwe z niewcisnietym przyciskiem
char letters[27]={'A',
'B',
'C',
'D',
'E',
'F',
'G',
'H',
'I',
'J',
'K',
'L',
'M',
'N',
'O',
'P',
'Q',
'R',
'S',
'T',
'U',
'V',
'W',
'X',
'Y',
'Z',
' ',
'0'};

const char **findNext(const char **codes, const char *beg) {
    int res;
    --codes;
    do {
        codes++;
        res=strncmp(*codes,beg,strlen(beg));
    } while(res!=0 && **codes!=0);
    return codes;
}

int main(int argc, char **argv) {
    const char *beg="1434";
    //printf("strcmp(%s,%s)=%d", codes[0],beg, strncmp(codes[0],beg,strlen(beg)));
    const char **i=codes;
    while (**(i=findNext(i, beg))!=*"\0") {
        printf("%s\n", *i+strlen(beg));
        ++i;
    }
    return 0;
}
