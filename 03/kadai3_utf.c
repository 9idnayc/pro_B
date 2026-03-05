#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>

//コマンドプロンプト、PowerShellを端末、あるいはVSCodeの端末上で使っている人は以下の3行のコメントを外すこと
#ifndef __linux__
#include <windows.h>
#endif

#define ADVANCED 1 //発展課題（絞り込み検索）に対応する場合は1に変更

#define DATAFILE "data_utf.csv"
#define CLEN 9 //郵便番号の最大バイト長
#define ALEN 200 //住所欄の最大バイト長
#define MAX_SIZE 200000//住所録中の住所数の最大数

struct Address {
  char code[CLEN];     // 郵便番号（7桁）
  char addr[ALEN];     // 住所（都道府県+市町村+町域）
};

struct Address table[MAX_SIZE];
long datasize = 0;

//動作確認で使うファイル実行モードで使う定数。修正不要
#define STANDBY_MAIN 0
#define QUERY_POSTAL 1
#define QUERY_ADDRESS 2
#define STANDBY_REFINE 3
#define QUERY_REFINE 4

int mode; //検索モード 0:なし，1:郵便番号検索，2:文字列検索
int refine_flag; //絞り込み検索の有無 0:なし，1:あり
char query[ALEN]; //検索クエリ（郵便番号or文字列）

long hit_index[MAX_SIZE];
long hit_size = 0;

//住所データファイルを読み取り，配列に保存
void scan(){
  FILE *fp;
  long line = 0;
  char code[CLEN+1],pref[ALEN+1],city[ALEN+1],town[ALEN+1];//tmp[ALEN+1];

  //datasizeの計算
  if ((fp = fopen(DATAFILE, "r")) == NULL) {
    fprintf(stderr,"error:cannot read %s\n",DATAFILE);
    exit(-1);
  }
  while(fscanf(fp, "%*[^,],%*[^,],\"%[^\"]\",%*[^,],%*[^,],%*[^,],\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",%*s", code, pref, city, town) != EOF ){
    
    // 郵便番号を保存
    strcpy(table[datasize].code, code);

    // 住所文字列を結合（都道府県 + 市町村 + 町域）
    table[datasize].addr[0] = '\0';
    strcat(table[datasize].addr, pref);
    strcat(table[datasize].addr, city);
    strcat(table[datasize].addr, town);

    datasize++;
    if(datasize >= MAX_SIZE) break;

    line++;
  }
  fclose(fp); 
}

double diff_time(clock_t t1, clock_t t2){
  return (double) (t2-t1)/CLOCKS_PER_SEC;
}

//初期化処理
void init(){
  clock_t t1,t2;

  t1 = clock();
  scan();
  t2 = clock();
  // printf("\n### %f sec for initialization. ###\n",diff_time(t1,t2)); 
  
}

//郵便番号による住所検索．検索結果を出力．
void code_search(){

  for(long i = 0; i < datasize; i++){

    if(strcmp(table[i].code, query) == 0){
        printf("%s:%s\n", table[i].code, table[i].addr);
    }
  }

  return;
}

//文字列による住所検索．検索結果を出力．
void address_search(){
  hit_size = 0;

  // 検索
  for(long i = 0; i < datasize; i++){

    if(strstr(table[i].addr, query) != NULL){
        hit_index[hit_size++] = i;
    }
  }

  // 郵便番号で昇順ソート（単純バブルソート）
  for(long i = 0; i < hit_size; i++){

    for(long j = i + 1; j < hit_size; j++){

      if(strcmp(table[hit_index[i]].code,
                table[hit_index[j]].code) > 0){
          long tmp = hit_index[i];
          hit_index[i] = hit_index[j];
          hit_index[j] = tmp;
      }
    }
  }

  // 出力
  for(long i = 0; i < hit_size; i++){
    long idx = hit_index[i];
    printf("%s:%s\n", table[idx].code, table[idx].addr);
  }

  return;
}

//絞り込み検索の実施
void refinement(){
  long next_hit_size = 0;
  long next_hit_index[MAX_SIZE];
  
  for(long i = 0; i < hit_size; i++){
    long real_index = hit_index[i];       
        
    if(strstr(table[real_index].addr, query) != NULL){
      next_hit_index[next_hit_size++] = real_index;
    }
  }

  hit_size = next_hit_size;
  
  for(long i = 0; i < hit_size; i++){
    hit_index[i] = next_hit_index[i];
  }

  for(long i = 0; i < hit_size; i++){
    long idx = hit_index[i];
    printf("%s:%s\n", table[idx].code, table[idx].addr);
  }

  return;
}

void input(){
  printf("\n"
	 "#########Top Menu#########\n"
	 "# Search by postal code: 1\n"
	 "# Search by address    : 2\n"
	 "# Exit                 : 0\n"
	 "> ");

  scanf("%d", &mode);
  if(mode == 1){
    printf("Postal code > ");
    scanf("%s", query);
  }else if(mode == 2){
    printf("Search String > ");
    scanf("%s", query);
  }
}

//絞り込み検索の有無を確認
void re_input(){
    printf("\n"
	 "# Continue Searching: 1\n"
	 "# Return to Top Menu: 0\n"
	 "> ");
  scanf("%d", &refine_flag);
  if(refine_flag == 1){
    printf("String for Refinement> ");
    scanf("%s", query);
  }
}

//クエリへの応答
void respond(){
  clock_t t1,t2;
  mode = 1;
  while(1){
    input();

    if(mode == 1){
      t1 = clock();
      code_search();
      t2 = clock();
      printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
    }
    else if(mode == 2){
      t1 = clock();
      address_search();
      t2 = clock();
      printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
      if(!ADVANCED) continue;
      while(1){
	re_input();
	if(refine_flag == 0) break;
	t1 = clock();
	refinement();
	t2 = clock();
	printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
      }
    }
    else break;
  }  
}

/*
動作確認時のファイル実行モード用の処理。
コマンド、検索対象の郵便番号、または住所の列をテキストファイルから受け取り、コード検索を行う。
このコードは変更しない。
*/
void run_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "cannot open test file: %s\n", filename);
        exit(1);
    }
    char buf[256];
    int state = STANDBY_MAIN;
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strcspn(buf, "\r\n")] = '\0';
        if (state == STANDBY_MAIN){
            int transit = atoi(buf);
            if (transit == 1){
                state = QUERY_POSTAL;
            }
            else if (transit == 2){
                state = QUERY_ADDRESS;
            }
            else if (transit == 0){
                exit(0);
            }
            else{
                printf("The current state is on STANDBY_MAIN. \n");
                exit(0);
            }
        }
        else if (state == QUERY_POSTAL){
            strcpy(query, buf);
            code_search();
            state = STANDBY_MAIN;
        }
        else if (state == QUERY_ADDRESS){
            strcpy(query, buf);
            address_search();
            state = STANDBY_REFINE;
        }
        else if (state == STANDBY_REFINE){
            int transit = atoi(buf);
            if (transit == 0){
                state = STANDBY_MAIN;
            }
            else if (transit == 1){
                state = QUERY_REFINE;
            }
            else{
                printf("The current state is on STANDBY_REFINE. \n");
                exit(0);
            }
        }
        else if (state == QUERY_REFINE){
            strcpy(query, buf);
            refinement();
            state = STANDBY_REFINE;
        }
        else{
            printf("Non-existing state occurs.\n");
            exit(0);
        }
    }
    fclose(fp);
}

int main(int argc, char **argv)
{
  #ifndef __linux__
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  #endif

  setvbuf(stdout, NULL, _IONBF, 0);
  init();
  if (argc > 1) {
     run_from_file(argv[1]);
  }
  else{
    respond();
  }
  return 0;
}
