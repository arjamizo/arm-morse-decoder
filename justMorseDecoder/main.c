#include <string.h>
#include <stdio.h>
const char *codes[29]=
{"1_111___\0",
"111_1_1_1___\0",
"111_1_111_1___\0",
"111_1_1___\0",
"1___\0",
"1_1_111_1___\0",
"111_111_1___\0",
"1_1_1_1___\0",
"1_1___\0",
"1_111_111_111___\0",
"111_1_111___\0",
"1_111_1_1___\0",
"111_111___\0",
"111_1___\0",
"111_111_111___\0",
"1_111_111_1___\0",
"111_111_1_111___\0",
"1_111_1___\0",
"1_1_1___\0",
"111___\0",
"1_1_111___\0",
"1_1_1_111___\0",
"1_111_111___\0",
"111_1_1_111___\0",
"111_1_111_111___\0",
"111_111_1_1___\0",
"111___\0",
"______\0",
"\0"};
char letters[27]={
'A',
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
0};

const char **findNext(const char **codes, const char *beg) {
    int res;
    --codes;
    do {
        codes++;
        res=strncmp(*codes,beg,strlen(beg));
    } while(res!=0 && **codes!=0);
    return codes;
}
typedef int u16;
char cntr[]={'-', '=', '#'};
void shotren(int x, int y, const char *t, u16 c, u16 bk) {
	static char buf[26];
	int p=0;
	buf[0]=0;
	const char *it=t;
	while(*it!=0) {
		char b=*it;
		int cnt=1;
		if(b=='1') {
			while(*(++it)==b && *it!=0 && cnt<3)
				cnt++;
			//if(b=='1')
				buf[p++]='0'+cnt;
		}
		else {
			buf[p++]=b;
            it++;
		}
		buf[p]=0;
		//if(*t!=0) t--;
	}
	//GUI_Text(x, y, buf, c, bk);
	printf("%s\n", buf);
}

int main(int argc, char **argv) {
    const char *beg="1";
    //printf("strcmp(%s,%s)=%d", codes[0],beg, strncmp(codes[0],beg,strlen(beg)));
    const char **i=codes;
    while (**(i=findNext(i, beg))!=0) {
        //printf(">%s\n", *i);
        shotren(0,0,*i,0,0);
        ++i;
    }
    return 0;
}
