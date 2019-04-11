#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "grep.h"
typedef void  (*SIG_TYP)(int);
#define BUFSIZE 100

int main(int argc, char *argv[]) {  //char *p1, *p2;
  if (argc != 3) { printf("Missing a regexp or file to be searched...\n"); return 0; } //checks if minimum requirements are input
  zero = (unsigned *)malloc(nlall * sizeof(unsigned));
  tfname = mkdtemp(tmpXXXXX);
  init();
  readfile(argv[2]);
  search_string(argv[1]);
  printf("\nquitting...\n");
  exit(1);
}

void regexp_buf_init(const char* regexp){
   strcpy(regexp_buf, regexp);
}

void readfile(const char* textfile){
  setnoaddr();
  if (vflag && fchange) { fchange = 0;  error(Q); }
  filename(textfile);
  init();
  addr2 = zero;
  if ((io = open((const char*)file, 0)) < 0) { lastc = '\n';  error(file); }
  setwide();
  squeeze(0);
  ninbuf = 0;
  append(getfile, addr2);
  exfile();
}

//************************************exfile closes file, prints the text file's character count and appends newline
void exfile(void) {
  close(io);
  io = -1;
  if (vflag) { putd();  puts_(" characters read"); }
}

void filename(const char* comm) {
  strcpy(savedfile, comm);
  strcpy(file, comm);
}

void search_string(const char* regexp){
  regexp_buf_init(regexp);
  char buf[GBSIZE];
  snprintf(buf, sizeof(buf), "/%s\n", regexp);
  grepline();
  printf("g%s", buf);
  const char* p = buf + strlen(buf) - 1;
  while (p >= buf) { ungetch_(*p--); }
  global(1);
}

void compile(int eof) {
  int c, cclcnt;
  char *ep = expbuf, *lastep, bracket[NBRA], *bracketp = bracket;
  if ((c = getchr()) == '\n') { peekc = c;  c = eof; }
  if (c == eof) {  if (*ep==0) { error(Q); }  return; }
  nbra = 0;
  if (c=='^') { c = getchr();  *ep++ = CCIRC; }
  peekc = c;
  lastep = 0;
  for (;;) {
    if (ep >= &expbuf[ESIZE]) { cerror(); }
    c = getchr();
    if (c == '\n') { peekc = c;  c = eof; }
    if (c==eof) {
      if (bracketp != bracket) { cerror(); }
      *ep++ = CEOF;
      return;
    }
    if (c!='*') { lastep = ep; }
    switch (c) {
      case '\\': if ((c = getchr())=='(') {
          if (nbra >= NBRA) { cerror(); }
          *bracketp++ = nbra;
          *ep++ = CBRA;
          *ep++ = nbra++;
          continue;
        }
        if (c == ')') {
          if (bracketp <= bracket) { cerror(); }
          *ep++ = CKET;
          *ep++ = *--bracketp;
          continue;
        }
        if (c>='1' && c<'1'+NBRA) { *ep++ = CBACK;
          *ep++ = c-'1';
          continue;
        }
        *ep++ = CCHR;
        if (c=='\n') { cerror(); }
        *ep++ = c;
        continue;
      case '.': *ep++ = CDOT;  continue;
      case '\n':  cerror();
      case '*':  if (lastep==0 || *lastep==CBRA || *lastep==CKET) { defchar(c, ep); }  *lastep |= STAR; continue;
      case '$':  if ((peekc=getchr()) != eof && peekc!='\n') { defchar(c, ep); }  *ep++ = CDOL;  continue;
      case '[':  *ep++ = CCL;  *ep++ = 0;  cclcnt = 1;  if ((c=getchr()) == '^') {  c = getchr();  ep[-2] = NCCL; }
        do {
          if (c=='\n') { cerror(); }
          if (c=='-' && ep[-1]!=0) {
            if ((c=getchr())==']') {
              *ep++ = '-';
              cclcnt++;
              break;
            }
            while (ep[-1] < c) {  *ep = ep[-1] + 1;
              ep++;
              cclcnt++;
              if (ep >= &expbuf[ESIZE]) { cerror(); }
            }
          }
          *ep++ = c;
          cclcnt++;
          if (ep >= &expbuf[ESIZE]) { cerror(); }
        } while ((c = getchr()) != ']');
        lastep[1] = cclcnt;
        continue;
    }
  }
}

void global(int k) {
  char *gp, globuf[GBSIZE];
  int c;
  unsigned int *a1;
  if (globp) { error(Q); }
  setwide();
  squeeze(dol > zero);
  if ((c = getchr()) == '\n') { error(Q); }
  compile(c);
  regexp_index = 0;
  gp = globuf;
  while ((c = getchr()) != '\n') {
     if (c == EOF) { error(Q); }
     if (c == '\\') {
       c = getchr();
       if (c != '\n') { *gp++ = '\\'; }
     }
     *gp++ = c;
     if (gp >= &globuf[GBSIZE-2]) { error(Q); }
  }
  if (gp == globuf) { *gp++ = 'p'; }
  *gp++ = '\n';
  *gp++ = 0;
  for (a1 = zero; a1 <= dol; a1++) {
    *a1 &= ~01;
    if (a1>=addr1 && a1<=addr2 && execute(a1)==k) { *a1 |= 01; }
   }
  for (a1 = zero; a1 <= dol; a1++) {
    if (*a1 & 01) {
      *a1 &= ~01;
      dot = a1;
      globp = globuf;
      print_genbuf();
      a1 = zero;
      break;
    }
  } grepline();
}

void print_genbuf(){  // prints the lines from genbuf that matches the regexp
  genp = genbuf;
  char tempbuf[BUFSIZE]; // buffer for regexp without '/' in the front of string
  char* temp;
  char linebuf_[LBSIZE]; // buffer to temporarily hold a line from genbuf (line = stops at '\n')
  char* lbufp;
  reverse_(regexp_buf); // reverse regexp because when debugging, it prints backwards
  rbufp = regexp_buf;
  ++rbufp; // skips the '/'
  for (temp = tempbuf; *rbufp != '\n'; rbufp++, temp++){
    *temp = *rbufp; // initializes tempbuf with regexp, excludes '/'
  }
  while (*genp != '\0') {
    for (lbufp  = linebuf_; *genp != '\n'; genp++, lbufp++){
      *lbufp = *genp; // temporarily stores a line from genbuf to linebuf_
    }
    if(strstr(linebuf_, tempbuf) != NULL) { // uses strstr to find if regexp matches anywhere in the linebuf_
      printf("%s\n", linebuf_); // then prints linebuf if there's a match
    }
    memset(linebuf_, '\0', sizeof(linebuf_)); // makes linebuf_ string empty to copy the next line from genbuf
    genp++;
  }
}

void reverse_(char* s){ // reverses input string for print_genbuf() function
  char* start = s;
  char* end = start + strlen(start) - 1;
  while (end > start) {
    char t = *end;
    *end-- = *start;
    *start++ = t;
  }
}

unsigned int* address(void) {
  int sign = 1, opcnt = 0, nextopand = -1, c;
  unsigned int *a, *b;
  a = dot;
  do {
    do c = getchr(); while (c==' ' || c=='\t');
    if ('0'<=c && c<='9') {
      peekc = c;
      if (!opcnt)  { a = zero; }
      a += sign*getnum();
    } else switch (c) {
      case '$':  a = dol;  /* fall through */
      case '.':  if (opcnt) { error(Q); } break;
      case '\'': c = getchr();
        if (opcnt || c<'a' || 'z'<c) { error(Q); }
        a = zero;
        do { a++; } while (a<=dol && names[c-'a']!=(*a&~01));
        break;
      case '?':  sign = -sign;  /* fall through */
      case '/': compile(c);  b = a;
        for (;;) { a += sign; if (a<=zero) { a = dol; }  if (a>dol) { a = zero; }  if (execute(a)) { break; }  if (a==b)  { error(Q); } }
        break;
      default: if (nextopand == opcnt) {  a += sign;  if (a < zero || dol < a)  { continue; } /* error(Q); */ }
        if (c!='+' && c!='-' && c!='^') {  peekc = c;  if (opcnt==0) { a = 0; }  return (a);  }
        sign = 1;  if (c!='+') { sign = -sign; }  nextopand = ++opcnt;  continue;
    }
    sign = 1;
    opcnt++;
  } while (zero<=a && a<=dol);
  error(Q);  /*NOTREACHED*/  return 0;
}
int advance(char *lp, char *ep) {  char *curlp;  int i;
  for (;;) {
    switch (*ep++) {
      case CCHR:  if (*ep++ == *lp++) { continue; } return(0);
      case CDOT:  if (*lp++) { continue; }    return(0);
      case CDOL:  if (*lp==0) { continue; }  return(0);
      case CEOF:  loc2 = lp;  return(1);
      case CCL:   if (cclass(ep, *lp++, 1)) {  ep += *ep;  continue; }  return(0);
      case NCCL:  if (cclass(ep, *lp++, 0)) { ep += *ep;  continue; }  return(0);
      case CBRA:  braslist[*ep++] = lp;  continue;
      case CKET:  braelist[*ep++] = lp;  continue;
      case CBACK:
        if (braelist[i = *ep++] == 0) { error(Q); }
        if (backref(i, lp)) { lp += braelist[i] - braslist[i];  continue; }  return(0);
      case CBACK|STAR:
        if (braelist[i = *ep++] == 0) { error(Q); }  curlp = lp;
        while (backref(i, lp)) { lp += braelist[i] - braslist[i]; }
        while (lp >= curlp) {  if (advance(lp, ep)) { return(1); }  lp -= braelist[i] - braslist[i];  }  continue;
      case CDOT|STAR:  curlp = lp;  while (*lp++) { }                star(lp, ep, curlp);
      case CCHR|STAR:  curlp = lp;  while (*lp++ == *ep) { }  ++ep;  star(lp, ep, curlp);
      case CCL|STAR:
      case NCCL|STAR:  curlp = lp;  while (cclass(ep, *lp++, ep[-1] == (CCL|STAR))) { }  ep += *ep;  star(lp, ep, curlp);
      default: error(Q);
    }
  }
}
// created function star to remove goto statements
int star(char *lp, char* ep, char* curlp) {
  do {  lp--;  if (advance(lp, ep)) { return(1); } } while (lp > curlp);  return(0); }

int append(int (*f)(void), unsigned int *a) {  unsigned int *a1, *a2, *rdot, *tl;  int nline;  nline = 0;  dot = a;
  while ((*f)() == 0) {
    if ((dol-zero)+1 >= nlall) {  unsigned *ozero = zero;  nlall += 1024;
      if ((zero = (unsigned *)realloc((char *)zero, nlall*sizeof(unsigned)))==NULL) {  error("MEM?");  onhup(0);  }
      dot += zero - ozero;  dol += zero - ozero;
    }
    *tl = putline();
    nline++;
    a1 = ++dol;
    a2 = a1+1;
    rdot = ++dot;
    while (a1 > rdot) { *--a2 = *--a1; }
    rdot = tl;
  }
  return(nline);
}
int backref(int i, char *lp) {
  char *bp;
  bp = braslist[i];
  while (*bp++ == *lp++) { if (bp >= braelist[i])   { return(1); } }
  return(0);
}
void blkio(int b, char *buf, long (*iofcn)(int, void*, unsigned long)) {
  lseek(tfile, (long)b*BLKSIZE, 0);
  if ((*iofcn)(tfile, buf, BLKSIZE) != BLKSIZE) {  error(T);  }
}

int cclass(char *set, int c, int af) {
  int n = *set++;
  if (c == 0) { return(0); }
  while (--n) { if (*set++ == c) { return(af); } }
  return(!af);
}
// created function defchar to remove goto statements
void defchar(int c, char *ep) { *ep++ = CCHR;  *ep++ = c; }
// created function cerror to remove goto statements
void cerror(){  expbuf[0] = 0;  nbra = 0;  error(Q); }

void error(char *s) {
  int c;  wrapp = 0;  listf = 0;  listn = 0; count = 0; pflag = 0;
  putchr_('?');
  puts_(s);
  lseek(0, (long)0, 2);
  if (globp) { lastc = '\n'; }
  globp = 0;
  peekc = lastc;
  if(lastc) { while ((c = getchr()) != '\n' && c != EOF) { } }
  if (io > 0) { close(io);  io = -1; }  longjmp(savej, 1);
}

int execute(unsigned int *addr) {  char *p1, *p2 = expbuf;
  int c;
  for (c = 0; c < NBRA; c++) {  braslist[c] = 0;  braelist[c] = 0;  }
  if (addr == (unsigned *)0) {
    if (*p2 == CCIRC) { return(0); }
    p1 = loc2;
  }
  else if (addr == zero) { return(0); }
  else { p1 = getline_blk(*addr); }
  if (*p2 == CCIRC) {  loc1 = p1;  return(advance(p1, p2+1)); }
  if (*p2 == CCHR) {    /* fast check for first character */
    c = p2[1];
    do {  if (*p1 != c) { continue; }
    if (advance(p1, p2)) {  loc1 = p1;  return(1); }
    } while (*p1++);
    return(0);
  }
  do {  /* regular algorithm */   if (advance(p1, p2)) {  loc1 = p1;  return(1);  }
  } while (*p1++);
  return(0);
}


char * getblock(unsigned int atl, int iof) {  int off = (atl<<1) & (BLKSIZE-1) & ~03, bno = (atl/(BLKSIZE/2));
  if (bno >= NBLK) {  lastc = '\n';  error(T);  }
  nleft = BLKSIZE - off;
  if (bno==iblock) {  ichanged |= iof;  return(ibuff+off);  }
  if (bno==oblock)  { return(obuff+off);  }
  if (iof==READ) {
    if (ichanged) { blkio(iblock, ibuff, (long (*)(int, void*, unsigned long))write); }
    ichanged = 0;
    iblock = bno;
    blkio(bno, ibuff, read);
    return(ibuff+off);
  }
  if (oblock>=0) { blkio(oblock, obuff, (long (*)(int, void*, unsigned long))write); }
  oblock = bno;
  return(obuff+off);
}
char inputbuf[GBSIZE];

int getchr(void) {  char c;
  if ((lastc=peekc)) {  peekc = 0;  return(lastc); }
  if (globp) {  if ((lastc = *globp++) != 0) { return(lastc); }
    globp = 0;
    return(EOF);
  }
  if ((c = getchr_()) <= 0) { return(lastc = EOF); }
}

int getchr_() {
  char c = (regexp_index > 0) ? regexp_buf[--regexp_index] : getchar();
  lastc = c&0177;
  return lastc;
}

void ungetch_(int c) {
  if (regexp_index >= BUFSIZE) {
    printf("ungetch: overflow\n");
  } else { regexp_buf[regexp_index++] = c; }
}

//**************************getfile() reads from the open file descriptor io into genbuf and stores the contents into char c ?
int getfile(void) {
  int c;
  char *lp = linebuf, *fp = nextip;   //*******************************fp = file pointer?
  do {
    if (--ninbuf < 0) {
      if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0) {
        if (lp>linebuf) { puts_("'\\n' appended");
          *genbuf = '\n';
        } else { return(EOF); }
      }
      fp = genbuf;
      while(fp < &genbuf[ninbuf]) {  if (*fp++ & 0200) { break; }  }
      fp = genbuf;
    }
    c = *fp++;  if (c=='\0') { continue; }  //****************************************************puts file read into genbuf into c string
    if (c&0200 || lp >= &linebuf[LBSIZE]) {  lastc = '\n';  error(Q);  }
    *lp++ = c;
    count++; // adds newline as last character
  } while (c != '\n');
  *--lp = 0;
  nextip = fp;
  return(0);
}
char* getline_blk(unsigned int tl) {
  char *bp, *lp;
  int nl = nleft;
  lp = linebuf;
  bp = getblock(tl, READ);
  tl &= ~((BLKSIZE/2)-1);
  while ((*lp++ = *bp++)) {
    if (--nl == 0) {  bp = getblock(tl+=(BLKSIZE/2), READ);  nl = nleft;  }
  }
  return(linebuf);
}
int getnum(void) { int r = 0, c;
  while ((c = getchr())>='0' && c <= '9') { r = r * 10 + c - '0'; }
  peekc = c;
  return (r);
}

void grepline(void) { //  puts_("------------------------------------ ");
  getchr();  // throw away newline after command
  for (int i = 0; i < 50; ++i) { putchr_('-'); }
  putchr_('\n');
}

void init(void) {  int *markp;
  close(tfile);
  tline = 2;
  for (markp = names; markp < &names[26]; )  {  *markp++ = 0;  }
  subnewa = 0;
  anymarks = 0;
  iblock = -1;
  oblock = -1;
  ichanged = 0;
  close(creat(tfname, 0600));
  tfile = open(tfname, 2);
  dot = dol = zero;
  memset(inputbuf, 0, sizeof(inputbuf));
}

void newline(void) {  int c;
  if ((c = getchr()) == '\n' || c == EOF) { return; }
  if (c == 'p' || c == 'l' || c == 'n') {  pflag++;
    if (c == 'l') { listf++;  }
    else if (c == 'n') { listn++; }
    if ((c = getchr()) == '\n') { return; }
  }  //error(Q);
}

void nonzero(void) { squeeze(1); }
void onhup(int n) {
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  if (dol > zero) {  addr1 = zero+1;
    addr2 = dol;
    io = creat("ed.hup", 0600);
    if (io > 0) { putfile(); }
  }
  fchange = 0;
  quit(0);
}
void onintr(int n) { signal(SIGINT, onintr);
  putchr_('\n');
  lastc = '\n';
  error(Q);
}

void print(void) {  unsigned int *a1 = addr1;
  nonzero();
  do {
    if (listn) {  count = a1 - zero;
      putd();
      putchr_('\t');
    }
    puts_(getline_blk(*a1++));
  }  while (a1 <= addr2);
  dot = addr2;
  listf = 0;
  listn = 0;
  pflag = 0;
}
void putchr_(int ac) {  char *lp = linp;  int c = ac;
  if (listf) {
    if (c == '\n') {
      if (linp != line && linp[-1]==' ') {  *lp++ = '\\';  *lp++ = 'n';  }
    } else {
      if (col > (72 - 4 - 2)) {  col = 8;  *lp++ = '\\';  *lp++ = '\n';  *lp++ = '\t';  }
      col++;
      if (c=='\b' || c=='\t' || c=='\\') {
        *lp++ = '\\';
        if (c=='\b') { c = 'b'; }
        else if (c=='\t') { c = 't'; }
        col++;
      }
      else if (c<' ' || c=='\177') {
        *lp++ = '\\';
        *lp++ =  (c>>6) +'0';
        *lp++ = ((c >> 3) & 07) + '0';
        c = (c & 07) + '0';
        col += 3;
      }
    }
  }
  *lp++ = c;
  if (c == '\n' || lp >= &line[64]) {  linp = line;
    write(oflag ? 2 : 1, line, lp - line);
    return;
  }
  linp = lp;
}

void putd(void) {  int r = count % 10;   //******* putd() counts and prints the number of characters in the text file
  count /= 10;
  if (count) { putd(); }
  putchr_(r + '0');
}

void putfile(void) {  unsigned int *a1;
  char *fp, *lp;
  int n, nib = BLKSIZE;
  fp = genbuf;
  a1 = addr1;
  do {
    lp = getline_blk(*a1++);
    for (;;) {
      if (--nib < 0) {
        n = (int)(fp-genbuf);
        if (write(io, genbuf, n) != n) {  puts_(WRERR);  error(Q);  }
        nib = BLKSIZE-1;
        fp = genbuf;
      }
      count++;  if ((*fp++ = *lp++) == 0) {  fp[-1] = '\n';  break; }
    }
  } while (a1 <= addr2);
  n = (int)(fp-genbuf);
  if (write(io, genbuf, n) != n) {  puts_(WRERR);  error(Q); }
}

int putline(void) {  char *bp, *lp;
  int nl = nleft;
  unsigned int tl;
  fchange = 1;
  lp = linebuf;
  tl = tline;
  bp = getblock(tl, WRITE);
  tl &= ~((BLKSIZE/2)-1);
  while ((*bp = *lp++)) {
    if (*bp++ == '\n') {  *--bp = 0;  linebp = lp;  break;  }
    if (--nl == 0) {  bp = getblock(tl += (BLKSIZE/2), WRITE);  nl = nleft;  }
  }
  nl = tline;
  tline += (((lp - linebuf) + 03) >> 1) & 077776;
  return(nl);
}
void puts_(char *sp) {  col = 0;   //******* puts_() prints the char* reference and puts a newline at the end of string
  while (*sp) { putchr_(*sp++); }
  putchr_('\n');
}
void quit(int n) {
  if (vflag && fchange && dol!=zero) {  fchange = 0;  error(Q);  }
  unlink(tfname);
  exit(0);
}

void setnoaddr(void) { if (given) { error(Q); } }
void setwide(void) { addr1 = zero + (dol>zero);  addr2 = dol; }
void squeeze(int i) { if (addr1 < zero+i || addr2 > dol || addr1 > addr2) { error(Q); } }
