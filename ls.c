#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct Node{
	char *f_name;
	struct Node *pNext;
};

struct List{
	struct Node *pHead;
};

int insertNode(struct List *list, char *d_name);
void printNode(struct List *list);

int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct Node *node;
	struct List list;

	list.pHead = NULL;
	dirp = opendir(".");

	while((dir=readdir(dirp))!=NULL){
		printf("%s\n", dir->d_name);
		insertNode(&list,dir->d_name);
	}

	printNode(&list);

	closedir(dirp);
	
	return 0;
}

int insertNode(struct List *list, char *d_name){
	struct Node *pNew = (struct Node*)malloc(sizeof(struct Node));
	pNew->f_name = (char*)malloc(sizeof(char)*strlen(d_name));
	memset(pNew->f_name, 0, sizeof(char)*strlen(d_name));
	pNew->f_name = d_name;
	struct Node *pCur=NULL;

	if(list->pHead==NULL){
		list->pHead = pNew;
	} else{
		pCur = list->pHead;
		while(pCur->pNext!=NULL){
			pCur = pCur->pNext;
		}
		pCur->pNext = pNew;
	}

	return 1;
}

void printNode(struct List *list){
	struct Node *pCur = list->pHead;

	printf("Print List\n");

	while(pCur!=NULL)
	{
		printf("%s\n", pCur->f_name);
		pCur = pCur->pNext;
	}
}
