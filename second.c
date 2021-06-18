#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "strbuf.h"
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include "queue.h"
#ifndef DEBUG
#define DEBUG 0
#endif

struct outDict{
	struct dictionary *wfd;
	int tokens;
};

struct dictionary{
	struct dictionary *next;
	char* word;
	double frequency;
};

int activeThreads = 0;

struct repo{
	char* file;
	int tokens;
	struct dictionary *dict;
	struct repo *next;
};

int j=0;
int k=0;
char* ss1=".txt";


struct repo *headRepo;
struct repo headRepo2;
queue_t dq, fq;
char arr2[100];
strbuf_t f={100,0,arr2};

int printQ(queue_t *q){
	for(int i=q->head;i<q->count+q->head;i++){
		printf("%d: Data: %s\n",i+1,q->data[i].path);
	}
	return 0;
}

struct dictionary *append(struct dictionary *head, struct dictionary *item){
	struct dictionary *cur = head;
	if(head->word==NULL){
		return item;
	}
	while(cur!=NULL){

		if(cur->next==NULL){
			cur->next=item;
			break;
		}
		else{
			cur=cur->next;
		}
	}
	return head;
}

int inList(struct dictionary *list, char* word){
	struct dictionary *cur = list;
	while(cur!=NULL){
		if(strcmp(cur->word,word)==0){
			return 1;
		}
		cur=cur->next;
	}
	return 0;
}

void printlist(char** list){
	int i=0;
	while(list[i][0]!='\0'){
		printf("%s",list[i]);
	}
}

void listdirs(strbuf_t d){
	int i=0;
	char delim[] = " ";
	char arr3[100];
	strbuf_t t2={100,0,arr3};
	sb_init(&t2,100);
  char* itr = strtok(d.data,delim);

	while(itr!=NULL){
		DIR *direct=opendir(itr);
		struct dirent *dt;

		while((dt=readdir(direct))){
			struct stat fs;
			char* x=malloc(sizeof(char)*1000);
			strcpy(x,itr);
			strcat(x,"/");
			strcat(x,dt->d_name);
			stat(x,&fs);
			if(S_ISDIR(fs.st_mode)){
				if(dt->d_name[0]=='.'){
					continue;
				}
				else{
					sb_concat(&t2,itr);
					sb_concat(&t2,"/");
					sb_concat(&t2,dt->d_name);
					sb_concat(&t2," ");
					char* c=malloc(sizeof(char)*1000);
					strcpy(c,itr);
					strcat(c,"/");
					strcat(c,dt->d_name);
					paths p;
					p.path=c;
					enqueue(&dq, p);
				}
			}
			else{
				if(dt->d_name[0]=='.'){
					continue;
				}
				else{
					if(strstr(dt->d_name, ss1) != NULL){
						sb_concat(&f,itr);
						sb_concat(&f,"/");
						sb_concat(&f,dt->d_name);
						sb_concat(&f," ");
						char* c=malloc(sizeof(char)*1000);
						strcpy(c,itr);
						strcat(c,"/");
						strcat(c,dt->d_name);
						paths p;
						p.path=c;
						//printf("\npath: %s",c);
						enqueue(&fq, p);
					}
				}
			}


		}

		itr = strtok(NULL, delim);
	}
	/*
	if(0==strcmp(t2.data,"")){
		return;
	}
	else{
		listdirs(t2);
		return;
	}
	*/
}

void printDict(struct dictionary *t){
	while(t!=NULL){
		printf("Word: %s, Frequency: %f\n",t->word,t->frequency);
		t=t->next;
	}
}

void printRepo(struct repo *r){
	while(r!=NULL){
		printf("\nFile: %s, Tokens: %d\n",r->file,r->tokens);
		printDict(r->dict);
		r=r->next;
	}
}

double KLD(struct dictionary *f, struct dictionary *mean){
	struct dictionary *curf = f;
	struct dictionary *curMean = mean;
	double kld = 0.0;
	while(curf!=NULL){
		curMean = mean;
		while(curMean!=NULL){
			if(strcmp(curf->word, curMean->word)==0){
				kld = kld + (curf->frequency*log2(curf->frequency/curMean->frequency));
			}
			curMean=curMean->next;
		}
		curf=curf->next;
	}
	return kld;
}

double JSD(double f1, double f2){
	return sqrt((0.5*f1)+(0.5*f2));
}

struct outDict *makeDict(char* path){
		pthread_mutex_lock(&fq.lock);

		FILE *txtFile;

    txtFile = fopen(path, "r");
    if(txtFile==NULL){
        perror("Error: ");
    }
    int length = 0;
    while(!(feof(txtFile))){
        fgetc(txtFile);
        length=length+1;
    }
    char* inputString = malloc(sizeof(char)*length);
    txtFile = fopen(path, "r");
    int index = 0;
    char c;
    while(!(feof(txtFile))){
        c = fgetc(txtFile);
        inputString[index]=c;
        index=index+1;
    }
    inputString[index-1]='\0';

	struct dictionary *head = malloc(sizeof(struct dictionary));
	char delim[] = " ,.:;\n\"";
	char* itr = strtok(inputString,delim);
	bool exists = true;
	double total = 0.0;
	int i = 0;
	while(itr!=NULL){
		i = 0;
		while(itr[i]!='\0'){
			itr[i]=toupper(itr[i]);
			i++;
		}
		exists = false;
		struct dictionary *newNode = malloc(sizeof(struct dictionary));
		newNode->word=itr;
		newNode->frequency=1;
		newNode->next=NULL;
		if(head->word==NULL){
			head->word=itr;
			head->frequency=1;
			head->next=NULL;
		}
		else{
			struct dictionary *cur = head;
			while(cur!=NULL){
				if(strcmp(cur->word,itr)==0){
					cur->frequency=cur->frequency+1;
					exists = true;
					break;
				}
				else{
					cur=cur->next;
				}
			}
			cur = head;
			if(!exists){
				struct dictionary *cur = head;
				while(cur!=NULL){
					if(cur->next==NULL){
						cur->next=newNode;
						break;
					}
					else{
						cur=cur->next;
					}
				}
			}
		}
		total = total + 1;
		itr = strtok(NULL, delim);
	}
	struct dictionary *cur = head;
	while(cur!=NULL){
		cur->frequency=cur->frequency/total;
		cur=cur->next;
	}
	struct outDict *out = malloc(sizeof(struct outDict));
	out->tokens=total;
	out->wfd=head;
	pthread_cond_signal(&fq.write_ready);
	pthread_mutex_unlock(&fq.lock);
	return out;

}

struct JSDpath{
	char* path;
	float jsd;
	int combinedTokens;
	struct JSDpath *next;
};

void printJSDpath(struct JSDpath *jsdpath){
	char arr5[100];
	strbuf_t a1={100,0,arr5};
	struct JSDpath *curr=jsdpath;
	int min=1000000;
	float minJSD=0.0;
	char* minPath=malloc(sizeof(char)*100);
	while(curr!=NULL){
		struct JSDpath *curr1=jsdpath;
		min=1000000;
		int counter2=0;
		while(curr1!=NULL){
			if(strstr(a1.data,curr1->path)==NULL){
				if(curr1->combinedTokens<min){
					strcpy(minPath,curr1->path);
					minJSD=curr1->jsd;
					min=curr1->combinedTokens;
				}
			}
			curr1=curr1->next;
		}
		printf("Path: %s, JSD: %f, \n",minPath,minJSD);
		sb_concat(&a1,minPath);
		curr=curr->next;
	}
}

struct JSDpath *appendjsd(struct JSDpath *head, struct JSDpath *item){
	struct JSDpath *cur = head;
	while(cur!=NULL){

		if(cur->next==NULL){
			cur->next=item;
			break;
		}
		else{
			cur=cur->next;
		}
	}
	return head;
}


struct dictionary *meanFreq(struct dictionary *f1, struct dictionary *f2){
	struct dictionary *cur1 = f1;
	struct dictionary *cur2 = f2;
	struct dictionary *head = malloc(sizeof(struct dictionary));

	while(cur1!=NULL){
		if(inList(f2, cur1->word)==0){
			struct dictionary *newNode = malloc(sizeof(struct dictionary));
			newNode->next=NULL;
			newNode->word=cur1->word;
			newNode->frequency=cur1->frequency/2.0;
			head = append(head, newNode);
		}
		else{
			cur2 = f2;
			while(cur2!=NULL){
				if(strcmp(cur1->word, cur2->word)==0){
					struct dictionary *newNode = malloc(sizeof(struct dictionary));
					newNode->next=NULL;
					newNode->word=cur1->word;
					newNode->frequency=(cur1->frequency+cur2->frequency)/2.0;
					head = append(head, newNode);
				}
				cur2=cur2->next;
			}
		}
		cur1=cur1->next;
	}

	struct dictionary *cur3 = f2;

	while(cur3!=NULL){
		if(inList(f1, cur3->word)==0){
			struct dictionary *newNode = malloc(sizeof(struct dictionary));
			newNode->next=NULL;
			newNode->word=cur3->word;
			newNode->frequency=cur3->frequency/2.0;
			head = append(head, newNode);
		}
		cur3=cur3->next;
	}

	return head;
}

void *fileEntry(void *ptr){
	while(fq.count!=0){
		char* path = dequeue(&fq, &fq.data[0], activeThreads);
		if(headRepo->file==NULL){
			headRepo->file=path;
			headRepo->tokens=makeDict(path)->tokens;
			headRepo->dict=makeDict(path)->wfd;
			headRepo->next=NULL;
		}
		else{
			struct repo *toAdd = malloc(sizeof(struct repo));
			toAdd->file=path;
			toAdd->tokens=makeDict(path)->tokens;
			toAdd->dict=makeDict(path)->wfd;
			toAdd->next=NULL;
			struct repo *cur = headRepo;
			while(cur!=NULL){
				if(cur->next==NULL){
					cur->next=toAdd;
					break;
				}
				cur=cur->next;
			}
		}
	}
	activeThreads--;
	return NULL;
}

void *dirEntry(void *ptr){
	while(dq.count!=0){
		char arr4[100];
		strbuf_t s={100,0,arr4};
		s.data = dequeue(&dq, &dq.data[0], activeThreads);
		listdirs(s);
	}
	activeThreads--;

	return NULL;

}

struct JSDpath *headnode;



void *analysisEntry(void *ptr){
	struct repo *cur1 = headRepo;
	struct repo *cur2 = headRepo;
	headnode=malloc(sizeof(struct JSDpath));
	headnode->path="";
	headnode->jsd=0;
	headnode->combinedTokens=0;
	headnode->next=NULL;
	while(cur1!=NULL){
		while(cur2!=NULL){
			char* path1=cur1->file;
			char* path2 = cur2->file;
			char* combine=malloc(100*sizeof(char));
			strcpy(combine,path1);
			strcat(combine," and ");
			strcat(combine,path2);
			struct JSDpath *midnode=malloc(sizeof(struct JSDpath));
			midnode->path=combine;
			midnode->jsd=JSD(KLD(cur1->dict,meanFreq(cur1->dict,cur2->dict)),KLD(cur2->dict,meanFreq(cur2->dict,cur1->dict)));
			midnode->combinedTokens=cur1->tokens+cur2->tokens;
			midnode->next=NULL;
			if(strcmp(path1,path2)!=0){
				if(headnode->combinedTokens==0){
					headnode->path=combine;
					headnode->jsd=JSD(KLD(cur1->dict,meanFreq(cur1->dict,cur2->dict)),KLD(cur2->dict,meanFreq(cur2->dict,cur1->dict)));
					headnode->combinedTokens=cur1->tokens+cur2->tokens;
					headnode->next=NULL;
				}
				else{
					appendjsd(headnode,midnode);
				}
				
			}			
			cur2=cur2->next;

		}
		cur1=cur1->next;
		cur2=cur1;	
	}

}

int main(int argc, char* argv[]){
	int i=1;
	int ft1, dt1, at1;
	ft1=1;
	dt1=1;
	at1=1;
	
	headRepo = malloc(sizeof(struct repo));

	init(&dq);
	init(&fq);

	char arr[100];
	strbuf_t d={100,0,arr};

	sb_init(&d,100);
	sb_init(&f,100);
	while(i!=argc){
		if(argv[i][0]=='-'){
			if(argv[i][1]=='f'){
				ft1=atoi(argv[i]+2);
			}
			if(argv[i][1]=='d'){
				dt1=atoi(argv[i]+2);
			}
			if(argv[i][1]=='a'){
				at1=atoi(argv[i]+2);
			}
			if(argv[i][1]=='s'){
				ss1=argv[i]+2;
			}
		}
		else{
			break;
		}
		i++;	
	}
	while(i!=argc){
		struct stat fs;
		stat(argv[i],&fs);
		if(S_ISDIR(fs.st_mode)){
			if(argv[i][0]=='.'){
				continue;
			}
			else{
				paths p;
				p.path=argv[i];
				enqueue(&dq, p);
				sb_concat(&d,argv[i]);
				sb_concat(&d," ");
			}

		}
		else{
			if(argv[i][0]=='.'){
				continue;
			}
			else{
				if(strstr(argv[i], ss1) != NULL){
					paths p;
					p.path=argv[i];
					enqueue(&fq, p);
					sb_concat(&f,argv[i]);
					sb_concat(&f," ");
				}
			}

		}
		i++;
	}
	//printf("\nstart\n");
	pthread_t dirThread;
	for(int j=0;j<dt1;j++){
		if(pthread_create(&dirThread, NULL, dirEntry, &dq)==0){
			activeThreads++;
			//printf("\nJust added, Current threads: %d\n", activeThreads);
		}
	}
	//pthread_join(dirThread, NULL);
	int wait = 0;
	int stopWait = 0;
	while(wait!=10000000){
		//printf("waiting here");
		if(activeThreads>0){
			wait++;
		}
		else{
			stopWait++;
			if(stopWait==1000){
				break;
			}
		}
		if(wait==9999999){
			//printf("thread woken here");
			pthread_cond_broadcast(&dq.write_ready);
			activeThreads--;
			break;
		}
		//printf("\nActive threads: %d\n",activeThreads);
	}
	//printf("\nActive threads final: %d\n",activeThreads);
	//pthread_join(dirThread, NULL);
	activeThreads = 0;
	pthread_t fileThread;
	/*
	pthread_create(&fileThread, NULL, fileEntry, &fq);
	activeThreads++;
	*/
	for(int j=0;j<ft1;j++){
		if(pthread_create(&fileThread, NULL, fileEntry, &fq)==0){
			activeThreads++;
		}
	}


	wait = 0;
	stopWait = 0;
	while(wait!=100000000){
		if(activeThreads>0){
			wait++;
		}
		else{
			stopWait++;
			if(stopWait==1000000){
				break;
			}
		}
		if(wait==99999999){
			pthread_cond_broadcast(&fq.write_ready);
			activeThreads--;
			break;
		}
	}
	//pthread_join(fileThread, NULL);
	pthread_t aThread;
	
	/*float jsd1=JSD(KLD(makeDict("test2/file1.txt")->wfd,meanFreq(makeDict("test2/file1.txt")->wfd,makeDict("test2/file4.txt")->wfd)),KLD(makeDict("test2/file4.txt")->wfd,meanFreq(makeDict("test2/file4.txt")->wfd,makeDict("test2/file1.txt")->wfd)));
	float jsd2=JSD(KLD(makeDict("test3/file5.txt")->wfd,meanFreq(makeDict("test3/file5.txt")->wfd,makeDict("test2/file4.txt")->wfd)),KLD(makeDict("test2/file4.txt")->wfd,meanFreq(makeDict("test2/file4.txt")->wfd,makeDict("test3/file5.txt")->wfd)));
	printf("\nJSD1: %f JSD2: %f",jsd1,jsd2);*/
	for(int j=0;j<at1;j++){
		if(pthread_create(&aThread, NULL, analysisEntry, NULL)==0){
			activeThreads++;
		}
	}
	pthread_join(aThread, NULL);
	printJSDpath(headnode);
	return 0;
}



int sb_init(strbuf_t *L, size_t length)
{
    L->data = malloc(sizeof(char) * length);
    if (!L->data) return 1;

    L->length = length;
    L->used   = 0;

    return 0;
}

void sb_destroy(strbuf_t *L)
{
    free(L->data);
}


int sb_append(strbuf_t *L, char item)
{
    if (L->used == L->length) {
	size_t size = L->length * 2;
	char *p = realloc(L->data, sizeof(char) * size);
	if (!p) return 1;

	L->data = p;
	L->length = size;

	if (DEBUG) printf("Increased size to %lu\n", size);
    }

    L->data[L->used] = item;
    ++L->used;

    return 0;
}


int sb_remove(strbuf_t *L, int *item)
{
    if (L->used == 0) return 1;

    --L->used;

    if (item) *item = L->data[L->used];

    return 1;
}

int sb_insert(strbuf_t *list, int index, char item){
	if(list->length==0){
		char *created=malloc(sizeof(char));
		if(!created){
			return 1;
		}
		list->data=created;
		list->length=1;
	}
	else if(list->length==list->used){
		list->length=list->length*2;
		size_t s=list->length;
		char *created=realloc(list->data,sizeof(char)*s);
		if (!created){
			return 1;
		}
		list->data=created;
	}
	if((index>list->length-1)||(index>list->used)){
		sb_append(list,item);
	}
	else{
		list->used++;
		for(int i = 0;i<list->used-index;i++){
			list->data[list->used-i]=list->data[list->used-i-1];
		}
		list->data[index]=item;

	}
	return 0;
}

int sb_concat(strbuf_t *sb, char *str){
	int i=0;
	while(str[i]!='\0'){
		sb_append(sb,str[i]);
		i=i+1;
	}
	return 0;
}
