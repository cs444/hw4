#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h>
#include<errno.h>
#include"mt19937ar.h"

typedef enum{ INSERT, SEARCH, DELETE, CAP } checker_t;
typedef enum{ ON, OFF } state;
sem_t sem_search;
sem_t sem_delete;
sem_t sem_insert;
state state_delete = OFF;
state state_insert = OFF;
state state_search = OFF;
state state_cap = OFF;
 pthread_mutex_t lockdisplay;


int search_count = 0;
int insert_count = 0;
int delete_count = 0;
int cap_count = 0;
int gc;
int sleep_time;

// singly linked list struct
struct node {
    int val;
    struct node *next;
}*head;


// declarations
void printer(struct node *);
 void printer_insert(struct node *);
void searcher(struct node *);
void inserter(struct node *);
void deleter(struct node *);
int check(checker_t checker);

/* *******************************************
 * Main
 * ******************************************/
int main(){
    int i,seed;
    init_genrand((unsigned long)&seed);
    struct node *head;
    head = malloc(sizeof(struct node));
    head->val = genrand_int32()%20;
    printer(head);
pthread_mutex_init(&lockdisplay, NULL);
    cap_count = search_count + insert_count + delete_count;
    // int sem_init(sem_t *sem, int pshared, unsigned int value)
    sem_init(&sem_search, 0, 0);
    sem_init(&sem_insert, 0, 0);
    sem_init(&sem_delete, 0, 0);

    pthread_t thread[7];    // 4 search 2 insert 1 delete
    pthread_create(&thread[0], NULL, (void *) &searcher, (void *) head);
    pthread_create(&thread[1], NULL, (void *) &searcher, (void *) head);
    pthread_create(&thread[2], NULL, (void *) &inserter, (void *) head);
    pthread_create(&thread[3], NULL, (void *) &inserter, (void *) head);
    pthread_create(&thread[4], NULL, (void *) &deleter, (void *) head);
    pthread_create(&thread[5], NULL, (void *) &searcher, (void *) head);
    pthread_create(&thread[6], NULL, (void *) &searcher, (void *) head);
    
    //pthread_create(&thread[5], NULL, (void *) &sll_del, (void *) head);
    pthread_join(thread[0],NULL);
    pthread_join(thread[1],NULL);
    pthread_join(thread[2],NULL);
    pthread_join(thread[3],NULL);
    pthread_join(thread[4],NULL);
    pthread_join(thread[5],NULL);
    pthread_join(thread[6],NULL);
    //pthread_join(thread[5],NULL);

    /* exit */  
    exit(0);
}

/* *******************************************
 * check function
 ********************************************/
int check(checker_t checker) { 
    int re=0;
    switch (checker) {
        case INSERT:
            if (state_delete == OFF && state_insert == OFF && state_cap == OFF) {   /* Inserter can only work with search. */
                sem_post(&sem_insert);
                re=1;
            }
            else 
                re=0;
            break;

        case SEARCH:                /* Searcher can work parallelly with search and insert, but cannot work with delete. */
            if (state_delete == OFF && state_cap == OFF) {
                sem_post(&sem_search);
                re=1;
            }
            else
                re=0;
            break;

        case DELETE:                /* Deleter can work with nobody. */
            if (state_delete == OFF && state_insert == OFF && search_count==0 && state_cap == OFF) {
                sem_post(&sem_delete);
                printf("search_count=%i\n",search_count);
                re=1;
            }
            else
                re=0;
            break;
        case CAP:                /* Capacity for part1. */
            
            sleep_time = rand() %5;
            sleep(sleep_time);
            cap_count = search_count+insert_count+delete_count;
            if (state_cap == OFF && cap_count >= 3) {
                state_cap = ON;
                pthread_mutex_lock(&lockdisplay);
                printf("========================================\n");
                
                printf("Now CLEAN UP!!!!\n More than 3 threads are actived: %d SEARCH | %d INSERT | %d DELETE\n",search_count,insert_count,delete_count);
                printf("Shut down, and wait for activated threads!!\n");
                
                printf("========================================\n");
                pthread_mutex_unlock(&lockdisplay);
                //while(cap_count != 0){
                cap_count=0;
                search_count=0;
                delete_count=0;
                insert_count=0;
                cap_count = search_count+insert_count+delete_count;
              state_delete = OFF;
              state_insert = OFF;
              state_search = OFF;
              state_cap = OFF;

                //}
                pthread_mutex_lock(&lockdisplay);
                printf("[Now RESTART].....%d SEARCH | %d INSERT | %d DELETE..........\n",search_count,insert_count,delete_count);
                printf("========================================\n");
                pthread_mutex_unlock(&lockdisplay);
                sleep(1);
                state_cap = OFF;
               
            }
             re=1;
             break;

        default:
             re=0;
            /*
            else{
                printf(".........cap count %d | s %d | i %d| d %d..........\n",cap_count,search_count,insert_count,delete_count);
            }
            */
            break;
    }

return re;
}


void searcher(struct node * list){ // list === head in main
    int c = 0, i, rdn, rt=3;
    struct node * cursor;
    while (1){
        
        sleep_time = rand() %5; /*Generate random number*/
        sleep(sleep_time);
        
        if(check(SEARCH) && check(CAP)) {
            sem_post(&sem_search);
            //sem_wait(&sem_delete);

            pthread_mutex_lock(&lockdisplay);
             search_count++;
            
            
             
            printf("SEARCHER WAKE UP --- [ACTIVED THREADS] %d SEARCH | %d INSERT | %d DELETE\n", search_count,insert_count,delete_count);
             pthread_mutex_unlock(&lockdisplay);
             
             rdn = genrand_int32()%20;
           pthread_mutex_lock(&lockdisplay);
        printf("Searcher is finding %d\n", rdn);
           pthread_mutex_unlock(&lockdisplay);
        cursor = list;
        if (cursor != 0){
            do {
                if (cursor->val != rdn){
                    cursor = cursor->next;
                    if(cursor == 0){
                           pthread_mutex_lock(&lockdisplay);
                        printf("%d not found.\n", rdn);
                           pthread_mutex_unlock(&lockdisplay);
                    }
                } else {
                       pthread_mutex_lock(&lockdisplay);
                    printf("Searcher found %d\n",rdn);
                    search_count--;
                       pthread_mutex_unlock(&lockdisplay);
                    break;          // only find one
                }
            } while (cursor != 0);
            
        } else {
            printf("The linked list is empty!!  Cannot find. \n");
            usleep(500000);
        }
        sleep(1);
     
               sleep_time= genrand_int32()%3;
        sleep(sleep_time);
    
        usleep(600000);

        }
       

    }
    pthread_exit(&rt);
}


void inserter(struct node * list){
    int c = 0, i, rdn, rt=1;
    struct node * cursor;
     while (1) {
         
         sleep_time = rand()%2 +3; /*Generate random number*/
         sleep(sleep_time);
         
    
        if (check(INSERT)&&check(CAP))
        { 
            sem_post(&sem_insert);
            
               pthread_mutex_lock(&lockdisplay);
               insert_count++;
            printf("++INSERTER WAKE UP ++ [ACTIVED THREADS] %d SEARCH | %d INSERT | %d DELETE\n", search_count,insert_count,delete_count);
               
              
        state_insert = ON;
      
         sleep_time= genrand_int32()%3;
         sleep(sleep_time);
        rdn = genrand_int32()%20;
         
        
         printf("++Insert %d ++\n", rdn);
          

            pthread_mutex_unlock(&lockdisplay);
            
        //printf("top of the list:%d\n", list->val);

         cursor = list;
        if (cursor != 0){
            while(cursor->next != NULL){
                cursor = cursor->next;
            }
            cursor->next = malloc(sizeof(struct node));
            cursor->next->val = rdn;
            
        } else {
               pthread_mutex_lock(&lockdisplay);
            printf("The linked list is empty!! ADD %d to list.\n",rdn);
               pthread_mutex_unlock(&lockdisplay);
            cursor = malloc(sizeof(struct node));
            cursor->val = rdn;
            cursor->next = NULL;
            
        }
        insert_count--;
        printer(list);
        sleep_time= genrand_int32()%5;
         sleep(sleep_time);
         
        //insert_count--;
        state_insert = OFF;
    
         sleep_time= genrand_int32()%3;
         sleep(sleep_time);
    
        
        sleep(1);
           
        }
       
        
     }
    pthread_exit(&rt);
}


void deleter(struct node * list){
    int cnt = 0,i, rdn;
    int rt=6; 
    struct node * cursor, * prev, *tmp;
    while (1) {
        
        sleep_time = rand() %5;
        sleep(sleep_time);
        
        if(check(DELETE)&&check(CAP)) {
             sem_post(&sem_delete);
             sem_wait(&sem_insert);
              sem_wait(&sem_search);
             while(search_count==0&&insert_count==0){
               pthread_mutex_lock(&lockdisplay);
              
            delete_count++;
            printf("--DELETER WAKE UP -- [ACTIVED THREADS] %d SEARCH | %d INSERT | %d DELETE\n", search_count,insert_count,delete_count);
             
             pthread_mutex_unlock(&lockdisplay);
          
           
        //sem_wait(&sem_delete);
        state_delete = ON;
        
        
        // count the length of the linked list
        cursor = list;
        while (cursor!=NULL) {
            ++cnt;
            cursor = cursor->next;
        }
        if (cnt == 0) {
               pthread_mutex_lock(&lockdisplay);
            printf("The linked list is empty!! nothing ot delete.\n");
               pthread_mutex_unlock(&lockdisplay);
        }
        else {
            // generate a random number and delete that node
            cursor = list;
            int seed;
            init_genrand((unsigned long)&seed);
            rdn = genrand_int32()%cnt;
               pthread_mutex_lock(&lockdisplay);
            printf("--Deleting index:%d---\n", rdn);
          
               pthread_mutex_unlock(&lockdisplay);
            //printf("cnt:%d\n",cnt);
            cnt = 0;
            if (rdn != 0) {             // check if we need to delete top
                //printf("Deleting index:%d\n",rdn);
                for (i=0; i<rdn; ++i) { // move the cursor to the position
                    prev = cursor;
                    cursor = cursor->next;
                }
                //printf("[Deleting index %d]\n",cursor->val,++rdn);
                prev->next = cursor->next;
                  delete_count--;
                //free(cursor);
                //cursor = prev->next;
            }
            else {
                if (list->next == 0) {
                    if (list != 0) {
                           pthread_mutex_lock(&lockdisplay);
                        printf("List is no  w empty.\n",rdn);

                           pthread_mutex_unlock(&lockdisplay);
                        free(list);
                        list = 0;
delete_count--;
                    }
                    else{
                           pthread_mutex_lock(&lockdisplay);
                        printf("List is now empty.\n");
                           pthread_mutex_unlock(&lockdisplay);
                           //delete_count--;
                }
                } 
                else {
                    //printf("The list is empty.\n");
                    list = list->next;
                    cursor = NULL;
                    //free(cursor);
                    cursor = list;
                    delete_count--;

                }
            }
        }
       printer(list);
   
break;
   }
        sleep_time= genrand_int32()%5;
        sleep(sleep_time);
        
        //delete_count--;
        state_delete = OFF;
  
        sleep_time= genrand_int32()%3;
        sleep(sleep_time);
        
        sleep(2);





        }
        
        
       
    }
    pthread_exit(&rt);
}

/* *******************************************
 * print all items in the list
 * ******************************************/
void printer(struct node * list){
    int rt=8,cnt=0,i;
    pthread_mutex_lock(&lockdisplay);
    printf("------------------------------------\n**Linked List**: ");
    pthread_mutex_unlock(&lockdisplay);
    struct node * cursor;
    cursor = list;
    if (cursor != 0){
        while (cursor!=NULL) {
            ++cnt;
            cursor = cursor->next;
        }
        //printf("cnt:%d\n",cnt);
        cursor = list;
        for (i=0;i<cnt;++i) {
               pthread_mutex_lock(&lockdisplay);
            printf(" %d ",cursor->val);
               pthread_mutex_unlock(&lockdisplay);
            cursor = cursor->next;
        }
           pthread_mutex_lock(&lockdisplay);
        printf("\n-------------------------------------\n");
           pthread_mutex_unlock(&lockdisplay);
    } else {
           pthread_mutex_lock(&lockdisplay);
        printf("-------------------------------------\nList currently empty\n----------------------\n");
          pthread_mutex_unlock(&lockdisplay);
        
    }
    
//exit(0);
}



void printer_insert(struct node * list){
    int rt=8,cnt=0,i;
   
    printf("------------------------------------\n**Linked List**: ");
   
    struct node * cursor;
    cursor = list;
    if (cursor != 0){
        while (cursor!=NULL) {
            ++cnt;
            cursor = cursor->next;
        }
        //printf("cnt:%d\n",cnt);
        cursor = list;
        for (i=0;i<cnt;++i) {
           
            printf(" %d ",cursor->val);
             
            cursor = cursor->next;
        }
          
        printf("\n-------------------------------------\n");
           
    } else {
          
        printf("-------------------------------------\nList currently empty\n----------------------\n");
         
        
    }
    
//exit(0);
}





