#define	BLKSIZE	4096
#define	FNSIZE	128
#define	LBSIZE	4096
#define	ESIZE	256
#define	NBRA	5
#define GBSIZE 256
#define CCHR 2
#define CDOT 4
#define CCL 6
#define CDOL 10
#define CEOF 11
#define NCCL 8
#define CBRA 1
#define CKET 12
#define CBACK 14
#define STAR 01
#define BUFSIZE 100

const int NBLK = 2047;
const int KSIZE = 9;
const int CCIRC = 15;
const int READ = 0;  const int WRITE = 1;  /* const int EOF = -1; */

int  peekc, lastc, given, ninbuf, io, pflag;
int  vflag  = 1, oflag, listf, listn, col, tfile  = -1, tline, iblock  = -1, oblock  = -1, ichanged, nleft;
int  names[26], anymarks, nbra, subnewa, subolda, fchange, wrapp, bpagesize = 20;
unsigned nlall = 128;  unsigned int  *addr1, *addr2, *dot, *dol, *zero;

FILE *fileptr; // for opening/closing files with fopen() and fclose()

char regexp_buf[BUFSIZE];  // buffer containing input regexp (string to search for)
char* rbufp;
int regexp_index = 0; //index of regexp buffer

long  count;
char  Q[] = "", T[] = "TMP";
char savedfile[FNSIZE]; // string of file[]'s contents?
char file[FNSIZE]; // string of file name i.e. "rings.txt"
char linebuf[LBSIZE], rhsbuf[LBSIZE/2], expbuf[ESIZE+4];
char  genbuf[LBSIZE], *nextip, *linebp, *globp, *mktemp(char *), tmpXXXXX[50] = "/tmp/eXXXXX";
char  *tfname, *loc1, *loc2, ibuff[BLKSIZE], obuff[BLKSIZE], WRERR[]  = "WRITE ERROR", *braslist[NBRA], *braelist[NBRA];
char  line[70];  char  *linp  = line;
void commands(void); //provides switch case of commands for grep/ed
unsigned int *address(void);  //for text file's line addresses
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);  int backref(int i, char *lp);
void blkio(int b, char *buf, long (*iofcn)(int, void*, unsigned long));
int cclass(char *set, int c, int af);  void compile(int eof);
void error(char *s);  int execute(unsigned int *addr);
void exfile(void); // closes file, calls putd (get/print character count) and adds a newline
void filename(const char* comm);  char *getblock(unsigned int atl, int iof); int getchr(void);
int getcopy(void);
int getfile(void);
char *getline_blk(unsigned int tl);  int getnum(void);
void global(int k);  //where grep command functions
void init(void);
void newline(void);  void nonzero(void);  void onhup(int n);
void onintr(int n);  char *place(char *sp, char *l1, char *l2);
void print(void);
void putchr_(int ac);
void putd(void);  // records and prints character count of text file
void putfile(void);
int putline(void);
void puts_(char *sp); // puts_ used like printf
void quit(int n);
void rdelete(unsigned int *ad1, unsigned int *ad2);  void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);  void setnoaddr(void);  void squeeze(int);  void substitute(int inglob);
jmp_buf  savej;
char grepbuf[GBSIZE];
void greperror(char);  void grepline(void);
void cerror();  // created function to remove goto statements
void defchar(int, char*); // created function to remove goto statements
int star(char *lp, char* ep, char* curlp); // created function to remove goto statements
void caseread(const char* c); // created function to remove goto statements
void readfile(const char* file);
void search_string(const char* regexp);
void regexp_buf_init(const char* regexp);
int getchr_();
void ungetch_(int c);
