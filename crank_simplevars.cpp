// Preamble generated by CrankLang
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
int  square(int  x);
std::string  digit_to_string(int  number);
int  crank_mainpoint_entry(int  argc, std::string  arguments[]);
int  square(int  x)
{
return x*x;
}
std::string  digit_to_string(int  number)
{
if (number==0) 
return "zero";

 else if (number==1) 
return "one";

 else if (number==2) 
return "two";

 else if (number==3) 
return "three";

 else if (number==4) 
return "four";

 else if (number==5) 
return "five";

 else if (number==6) 
return "six";

 else if (number==7) 
return "seven";

 else if (number==8) 
return "eight";

 else if (number==9) 
return "nine";











}
int  crank_mainpoint_entry(int  argc, std::string  arguments[])
{
int  i = 0;
;printf("hello world! There are %d arguments\n", argc);int  j = 0;
;while (j<25) 
{
printf("%d\n", square(j));j+=1;}

printf("5 + 10 * 4 - 5 = %d\n", 5+10*4-5);if (j==25) 
{
printf("not sure if this works\n");}

 else {
printf("maybe\n");}

{
int  number = 987245;
;printf("We're going to dissect the number \"%d\"\n", number);while (number>0) 
{
int  digit = {};
;digit=number%10;printf("%s", digit_to_string(digit).c_str());number/=10;if (number>0) 
printf("-");

}

printf("\n");}
while (i<argc) 
{
printf("argument: %s\n", arguments[i].c_str());i+=1;}

return 0;
}

/**
   This file is copy and pasted wholesale,

   DO NOT TOUCH ME!

   ASSUMED std::string is usable, and vector!
**/

int main(int argc, char** argv) {
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    return crank_mainpoint_entry(arguments.size(), arguments.data());
}
