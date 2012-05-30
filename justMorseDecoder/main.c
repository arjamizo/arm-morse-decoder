#include <stdio.h>
#include <string.h>
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

void append(char* s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}




char morseBuf[256];
char buf[200];

int printTable(char *morseBuf) {
int i; i=0;
    #ifdef STM32F103VC
    LCD_DrawSquare(4,20,240, 13*15,Black);
    #endif
int ok=-2; //nie ma zadnej nadzei na znalezienie rozwiazania
for(i=0; i<26; ++i)
{
	int x=(i/14)*120;
	int y=(i%14)*15+20;
	buf[0]=letters[i];
	buf[1]=0;
	int res=strlen(morseBuf)==0 || strncmp(morseBuf, codes[i], strlen(morseBuf))==0;
	int ncmp=(strcmp(morseBuf, codes[i])==0);
	if(res && ncmp) ok=i; //i-ty element jest rozwiazaniem
	if(res && ok<0) ok=-1; //jest nadal szansa
	int werdykt=(res!=0) || (ncmp!=0);
    int j=3;
	#ifdef STM32F103VC
		GUI_Text(x+4, y, buf, werdykt?White:Black, Black);
	#endif
	strncpy(buf, codes[i]+((strlen(morseBuf)==0)?0:(strlen(morseBuf))), j);
	//buf[j++]='0'+werdykt;
	//buf[j++]='0'+ncmp;
	//buf[j++]='0'+res;
	buf[j++]=0;
    #ifdef STM32F103VC
    GUI_Text(x+23, y, buf, werdykt?White:Black, Black);
    #else
	printf("werdykt=%d>letter= >%s< normalcmp=%d cmp=%d morsebuflen=%d codes[i]len=%d<",werdykt,buf,ncmp, res,strlen(morseBuf),strlen(codes[i]));
	if(werdykt){
		printf("%s",buf);
        }
	printf("\n");
	#endif
}
return ok;
}


int main(int argc, char **argv) {
    strcpy(morseBuf, "1_11");
    int r;
    r=printTable(morseBuf);
    printf("r=%d => %c\n",r,letters[r]);
    return 0;
}
/* strncpy example */
#include <stdio.h>
#include <string.h>

int amain ()
{
  char str1[]= "To be or not to be";
  char str2[6];
  strncpy (str2,str1,5);
  str2[5]='\0';
  puts (str2);
  return 0;
}
