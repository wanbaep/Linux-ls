#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct Node{
	char *d_name;
	struct Node *pNext;
	int hidden;
};

struct List{
	struct Node *pHead;
};

int insert_node(struct List *list, struct dirent *dir);
void add_hidden_mark(struct List *list);
void print_node(struct List *list);

int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct Node *node;
	struct List list;
	int hidden, in_flag, c_p;

	list.pHead = NULL;
	dirp = opendir(".");

	
	while((dir=readdir(dirp))!=NULL){
		insert_node(&list, dir);
	}
	add_hidden_mark(&list);
	print_node(&list);

	closedir(dirp);
	
	return 0;
}

int insert_node(struct List *list, struct dirent *dir){
	struct Node *pCur = NULL, *pNext = NULL, *pPre =NULL;	
	struct Node *pNew = (struct Node*)malloc(sizeof(struct Node));
	
	pNew->d_name = (char*)malloc(sizeof(char)*strlen(dir->d_name));
	memset(pNew->d_name, 0, sizeof(char)*strlen(dir->d_name));

	if(dir->d_name[0]=='.')
	{
		strcpy(pNew->d_name,dir->d_name+1);
		pNew->hidden = 1;
	}
	else
	{
		strcpy(pNew->d_name,dir->d_name);
		pNew->hidden = 0;
	}
	
	if(list->pHead == NULL)
	{
		list->pHead = pNew;
		pNew->pNext = NULL;
	}
	else
	{
		pCur = list->pHead;
		pPre = pCur;
		while(pCur!=NULL)
		{
			if(strcasecmp(pCur->d_name,pNew->d_name) > 0)
			{
				if(pCur == list->pHead)
				{
					pNew->pNext = pCur;
					list->pHead = pNew;
				}
				else
				{
					pPre->pNext = pNew;
					pNew->pNext = pCur;
				}
				return 1;
			}
			else
			{
				pPre = pCur;
				pCur = pCur->pNext;
			}
		}
		pPre->pNext = pNew;
	}

	return 1;
}

void add_hidden_mark(struct List *list){
	struct Node *pCur = list->pHead;
	int i;
	char temp[100]={"\0"};

	while(pCur!=NULL)
	{
		if(pCur->hidden==1)
		{
			memset(temp,0,sizeof(char)*100);
			strcpy(temp,pCur->d_name);
			pCur->d_name[0]='.';
			for(i=0;i<strlen(temp);i++)
			{
				pCur->d_name[i+1]=temp[i];
			}
		}
		pCur = pCur->pNext;
	}

}

void print_node(struct List *list){
	struct Node *pCur = list->pHead;

	while(pCur!=NULL)
	{
		printf("%s\n", pCur->d_name);
		pCur = pCur->pNext;
	}
}
