#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "MT.h"

//大域変数
long num, maxGen;


/*
  娘を出産する母親を決定．
  現役世代のi番目の女性が母親となる
  編集厳禁．ここ以外でgenrand_*を使用しない．  
*/
long nextMother(){
  return (long) (genrand_real3()*num);
}

int main(int argc, char **argv){
  long times=0;
  clock_t t1,t2;
  char *th;

  /*******初期設定*******/
  if(argc != 4){
    printf("this program requires exactly three arguments\n");
    printf("arg1 = num, arg2 = max_gen, arg3 = seed\n");
    exit(-1);
  }
  num = atol(argv[1]); //1世代の人口N．大域変数．
  maxGen = atol(argv[2]); //世代数の上限G．大域変数．
  int seed = atoi(argv[3]);
  init_genrand(seed);//乱数生成の初期化．【編集厳禁!】
  t1 = clock();
  /********** ***********/

  /*
  ミトコンドリアイブを見つけるたびに下記要領で出力を行う．
  eveGenはミトコンドリア・イブの世代番号（0から始まる）を示し， eveIDはその世代のなかでのミトコンドリア・イブのIDを表す．
  foundGenはミトコンドリア・イブが出現した世代を表す．
  */  
  times++;
  if((times % 10 == 1) && (times % 100 !=11)){
    th = "st";
  }else if((times % 10 == 2) && (times % 100 !=12)){
    th = "nd";
  }else if((times % 10 == 3) && (times % 100 !=13)){
    th = "rd";
  }
  else{
    th = "th";
  }
  printf("The %ld%s Eve (%ld,%ld) occurs at %ld th generation\n", times,th,eveGen,eveId,foundGen);


  /*
    最後にミトコンドリアイブ の出現頻度を出力する．
    timesはミトコンドリアイブ の出現回数を記憶する変数． 
  */

  printf("Eve occurrence rate is %f\n",(double) times/maxGen);

  t2 = clock();
  printf("Done within ### %f sec . ###\n", (double) (t2-t1)/CLOCKS_PER_SEC);
  
  return 0;
}
