#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
int token;
char *src,*old_src;
int poolsize;
int line;
int token_val;//value of current token (mainly for num)
int *current-i的，  //current parsed ID
    *symbols; //symbol tables

//fileds of identifier
enum{Token,Hash,Name,Type,Class,Value,BType,BClass,BValue,IdSize};

int *text,//text segment
    *old_text,//for dump text segment
    *stack;//stack
char *data;//data segement
int *pc,*sp,*bp,ax,cycle;//virtual machine registers

enum {           //标记流的标记符
      Num = 128, Fun, Sys, Glo, Loc, Id,
      Char, Else, Enum, If, Int, Return, Sizeof, While,
      Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
//types of variable/function
enum {CHAR,INT,PTR};
int* idmain;//    the  'main' function

//instruments
enum {LEA,IMM,JMP,CALL,JZ,JNZ,ENT,ADJ,LEV,LI,LC,SI,SC,PUSH,
      OR,XOR,AND,EQ,NE,LT,GT,LE,GE,SHL,SHR,ADD,SUB,MUL,DIV,MOD,
      OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT};
//词法分析器j
void next()
{
   char *last_pos;
   int hash;
   while(token=*src){
       ++src;
       // parse token here
       if(token=='\n')
           ++line;
       else if(token=='#')
           //skip macro ,because we will not support it
           while(*src!=0&&*src!='\n')
           {
               src++;
           }
       

//识别标识符
       else if((token>='a'&&token<='z')||(token>= 'A'&&token<='Z')||(token=='_'))  //parse identifier
       {
           last_pos=src-1;
           hash=token;
           while((*src>='a'&&*src<='z')||(*src>='A'&&*src<='Z')||(*src=='_')||(*src>='0'&&*src<='9'))
           {
               hash=hash*147+*src;
               src++;
           }
           //look for existing identifier,linear search
           current_id=symbols;
           while(current_id[Token])     //symbols是一个已经存在的表，获得表头，遍历循环，查找是否有重复
           {
               if(current_id[Hash]==hash&&!memcmp((char*)current_id[Name],last_pos,src-last_pos))
               {
                   //found one,return
                   token=current_id[Token];
                   return;
               }
               current_id+=IdSize;
           }        //////////////////////////////查找结束
           //store new ID
           current_id[Name]=(int)last_pos;
           current_id[Hash]=hash;
           token=current_id[Token]=Id;
           return;
       } 




//识别数字
       else if(token>='0'&&token<='9'){
           //parse number,three kinds,,dec hex(0xf),dec (017)
           token_val=token-'0';
           if(token_val>0)
           {
               //dec,
               while(*src>='0'&&*src<='9')
                   token_val=token_val*10+*src++-'0';
           }
           else if(*src=='x'||*src=='X')
           {
               //hex
               token=*++src;
               while((token>='0'&&token<='9')||(token>='a'&&token<='f')||(token>='A'&&token<='F'))
               {
                   token_val=token_val*16+(token & 0xf)+(token>'A'?9:0);
                   token=*++src;
               }
           }
           else
           {
               //oct
               while(*src>='0'&&*src<='7')
                   token_val=token_val*8+*src++-'0';
           }
           token=Num;
           return;
       }


//识别字符串
        else if(token=='"'||token=='\'')
        {
            //parse string literal ,only \n is supported ,store the string literal into data
            last_pos=data;
            while(*src!=0&&*src!=token)
            {
                token_val=*src++;
                if(token_val=='\\')
                        //escape character
                {
                    token_val=*src++;
                    if(token_val=='n')
                        token_val='\n';
                }
                if(token=='"')     //////////表示这是一个字符串
                    *data++=token_val;
            }
            src++;
            //if it is a single character ,return Num token
            if(token=='"')    //字符串返回数据地址
                token_val=(int)last_pos;
            else
                token=Num;
        }




//识别注释
        else if(token=='/')
        {
            if(*src=='/')      //跳过注释段
                while(*src!=0&&*src!='\n')
                    ++src;
            else
            {                //不是注释，那就是除号，，向前看一位
                token=Div;
                return;
            }
        }



//其他词法解析，基本上如果有歧义，将向下看一位
        else if(token=='=')
        {
            if(*src=='=')
            {
                src++;
                token=Eq;
            }
            else
                token=Assign;
            return;
        }

       else if(token=='+')
       {
           if(*src=='+')
           {
               src++;
               token=Inc;
           }
           else
               token=Add;
           return;
       }
      
       else if(token=='-')
       {
           if(*src=='-')
           {
               src--;
               token=Dec;
           }
           else
               token=Sub;
           return;
       }
       
       else if(token=='!')
       {
           if(*src=='=')
           {
               src++;
               token=Ne;
           }
           return;
       }

       else if(token=='<')
       {
           if(*src=='=')
           {
               src++;
               token=Le;
           }
           else if(*src=='<')
           {
               src++;
               token=Shl;
           }
           else
               token=Lt;
           return;
       }

        else if(token=='>')
       {
           if(*src=='=')
           {
               src++;
               token=Ge;
           }
           else if(*src=='<')
           {
               src++;
               token=Shr;
           }
           else
               token=Gt;
           return;
       }

       else if(token=='|')
       {
           if(*src=='|')
           {
               src++;
               token==Lor;
           }
           else 
               token=Or;
           return;
       }
       
       else if(token=='&')
       {
           if(*src=='&')
           {
               src++;
               token=Lan;
           }
           else
               token=And;
           return;
       }

       else if(token=='^')
       {
           token=Xor;
           return;
       }
       else if(token=='%')
       {
           token=Mod;
           return;
       }
       else if(token=='*')
       {
           token=Mul;
           return;
       }
       else if(token=='[')
       {
           token=Brak;
           return;
       }
       else if(token=='?')
       {
           token=Cond;
           return;
       }
       else if(token=='~'||token==';'||token=='{'||token=='(||token=='')'||token==']'||token==','||token==':')
           //directly return the character as token;
           return;
   } 
   return 0;
}

void expression(int level)
{
    //do something
}
//语法分析
void program()
{
    next();
    while(token>0)
    {
        printf("token is:%c\n",token);
        next();
    }
}
//虚拟机的入口
int eval()
{
    //do nothing yet
    int op,*tmp;
    while(1)
    {
        op=*pc++;//得到第一个命令，并把pc指向下一个命令

        if (op==IMM) { ax=*pc++;}
        else if(op==LC) {ax=*(char*)ax;}
        else if(op==LI) {ax=*(int*)ax;}
        else if(op==SC) {ax=*(char*)*sp++=ax;}
        else if(op==SI) {*(int*)*sp++=ax;}
        else if(op==PUSH) {*--sp=ax;}
        else if(op==JMP) {pc=(int*)*pc;}
        else if(op==JZ) {pc=ax ? pc+1:(int*)*pc;}
        else if(op==JNZ) {pc=ax ? (int*)*pc:pc+1;}
        else if(op==CALL) {*--sp=(int)(pc+1);pc=(int*)*pc;}
        //else if(op==RET) {pc=(int*)*sp++;}
        else if(op==ENT) {*--sp=(int)bp;bp=sp;sp=sp-*pc++;}
        else if(op==ADJ) {sp=sp+*pc++;}
        else if (op==LEV) {sp=bp;bp=(int*)*sp++;pc=(int*)*sp++;}
        else if(op==LEA) {ax=(int)(bp+ *pc++);}
        
        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
		else if (op == MOD) ax = *sp++ % ax;

        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
        else if(op==OPEN) {ax=open((char*)sp[1],sp[0]);}
        else if(op==CLOS) {ax=close(*sp);}
        else if(op==READ) {ax=read(sp[2],(char*)sp[1],sp[0]);}
        else if(op==PRTF) {tmp=sp+pc[1];ax=printf((char*)tmp[-1],tmp[-2],tmp[-3],tmp[-4],tmp[-5],tmp[-6]);}
        else if(op==MALC) {ax=(int)malloc(*sp);}
        else if(op==MSET) {ax=(int)memset((char*)sp[2],sp[1],*sp);}
        else if(op==MCMP) {ax=memcmp((char*)sp[2],(char*)sp[1],*sp);}
        else
        {
            printf("unknown instruction:%d\n",op);
            return -1;
        }
    }

    return 0;
}

int main(int argc,char **argv)
{
    int i,fd;

    argc--;
    argv++;

    poolsize=256*1024;
    line=1;
    
    src="char else enum if int return sizeof while "
        "open read close printf malloc memset memcmp exit void main";
    //add keywords to symbol table
    i=Char;
    while(i<=While)
    {
        next();
        current_id[Token]=i++;
    }
    //add library to symbol table
    i=OPEN;
    while(i<=EXIT)
    {
        next();
        current_id[Class]=Sys;
        current_id[Type]=INT;
        current_id[Value]=i++;
    }
    next();current_id[Token]=Char;//handle void type
    next();idmain=current_id;//keep track of main
    
    


    if((fd=open(*argv,0))<0)   //读取文件
    {
        printf("could not open(%s)\n",argv);
        return -1;
    }
    
    if(!(src=old_src=malloc(poolsize)))
    {
        printf("could not malloc for source area\n");
        return -1;
    }

    if((i=read(fd,src,poolsize-1))<=0)
    {
        printf("read return %d\n",i);
        return -1;
    }

    src[i]=0;//add EOF character
    close(fd);

//allocate memory for virtual machine
    if (!(text=old_text=malloc(poolsize)))
    {
        printf("could malloc memory for text area\n");
        return -1;
    }
    
    if (!(data=malloc(poolsize)))
    {
        printf("could malloc memory for data area\n");
        return -1;
    }
    
    if (!(stack=malloc(poolsize)))
    {
        printf("could malloc memory for stack area\n");
        return -1;
    }
    memset(text,0,poolsize);
    memset(data,0,poolsize);
    memset(stack,0,poolsize);
    bp=sp=(int*)((int)stack+poolsize);
    ax=0;

    i=0;
    text[i++]=IMM;
    text[i++]=10;
    text[i++]=PUSH;
    text[i++]=IMM;
    text[i++]=20;
    text[i++]=ADD;
    text[i++]=PUSH;
    text[i++]=EXIT;
    pc=text;

    program();
    return eval();
}
