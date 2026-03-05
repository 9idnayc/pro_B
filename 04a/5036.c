#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "MT.h"

//大域変数
long num, maxGen;

/*
Code 1
*/
typedef struct Person {
    long id;
    long generation;
    struct Person *mother;
    int ref_count;
} Person;

/*
  娘を出産する母親を決定．
  現役世代のi番目の女性が母親となる
  編集厳禁．ここ以外でgenrand_*を使用しない．  
*/
long nextMother(){
  return (long) (genrand_real3()*num);
}

/*
Code 2
*/
Person* createPerson(long id, long gen, Person *mother) {
    Person *p = (Person *)malloc(sizeof(Person));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    p->id = id;
    p->generation = gen;
    p->mother = mother;
    p->ref_count = 1;
    
    if (mother != NULL) {
        mother->ref_count++;
    }
    return p;
}

void releasePerson(Person *p) {
    if (p == NULL) return;
    
    p->ref_count--;
    if (p->ref_count == 0) {
        // Recursive step: release the mother before freeing self
        releasePerson(p->mother);
        free(p);
    }
}

// Lowest Common Ancestor
Person* getLCA(Person *a, Person *b) {
    if (a == NULL || b == NULL) return NULL;
    
    // Align generations first (move the younger one up)
    while (a->generation > b->generation) {
        a = a->mother;
        if (a == NULL) return NULL;
    }
    while (b->generation > a->generation) {
        b = b->mother;
        if (b == NULL) return NULL;
    }
    
    // Move both up until they meet
    while (a != b) {
        a = a->mother;
        b = b->mother;
        if (a == NULL || b == NULL) return NULL;
    }
    
    return a;
}

int main(int argc, char **argv){
  long times=0;
  clock_t t1,t2;
  char *th;

  /*
  Code 3
  */
  long i, gen;
  Person **current_pop, **next_pop;
  Person *current_eve = NULL, *prev_eve = NULL;

  long eveGen = 0, eveId = 0, foundGen = 0;

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
  Code 4
  */
  current_pop = (Person **)malloc(sizeof(Person*) * num);
  next_pop = (Person **)malloc(sizeof(Person*) * num);
  
  if (!current_pop || !next_pop) {
    exit(1);
  }

  // Initialize Gen 0
  for (i = 0; i < num; i++) {
    current_pop[i] = createPerson(i, 0, NULL);
  }

  // Main Simulation Loop
  for (gen = 1; gen < maxGen; gen++) {
      
    // 1. Create Next Generation
    for (i = 0; i < num; i++) {
        long mom_idx = nextMother();
        next_pop[i] = createPerson(i, gen, current_pop[mom_idx]);
    }

    // 2. Find Mitochondrial Eve for this generation
    // Start with the first person's ancestor as the candidate
    current_eve = next_pop[0]; 
      
    // Calculate LCA with every other person in the population
    for (i = 1; i < num; i++) {
        current_eve = getLCA(current_eve, next_pop[i]);
        if (current_eve == NULL) break; // No common ancestor yet
    }

    // 3. Check if Eve has changed or appeared
    if (current_eve != NULL) {
        int is_new_eve = 0;
        
        if (prev_eve == NULL) {
            is_new_eve = 1; // First time Eve appears
        } else if (current_eve != prev_eve) {
            is_new_eve = 1; // Eve has changed
        }

        if (is_new_eve) {
        // Update tracking variables
        prev_eve = current_eve;
        eveGen = current_eve->generation;
        eveId = current_eve->id;
        foundGen = gen;
             
            // 4. Output
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
        }
    }

    // 5. Cleanup: Release current generation
    for (i = 0; i < num; i++) {
        releasePerson(current_pop[i]);
    }

    // Swap pointers for next iteration
    Person **temp = current_pop;
    current_pop = next_pop;
    next_pop = temp;
  }
  
  // Final cleanup for the last generation
  for (i = 0; i < num; i++) {
      releasePerson(current_pop[i]);
  }
  free(current_pop);
  free(next_pop);

  /*
    最後にミトコンドリアイブ の出現頻度を出力する．
    timesはミトコンドリアイブ の出現回数を記憶する変数． 
  */

  printf("Eve occurrence rate is %f\n",(double) times/maxGen);

  t2 = clock();
  printf("Done within ### %f sec . ###\n", (double) (t2-t1)/CLOCKS_PER_SEC);
  
  return 0;
}
