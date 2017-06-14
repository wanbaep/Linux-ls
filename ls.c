#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

struct Node{
	char *d_name;
	struct Node *pNext;
	struct Node *pQnext;
	int hidden;
};

struct List{
	struct Node *pHead;
};

struct Queue{
	struct Node *pHead;
	struct Node *pTail;
};

struct Queue queue;
char *mon_type[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int insert_node(struct List *list, struct dirent *dir);
void add_hidden_mark(struct List *list);
void select_list(struct List *list, int aflag);
int create_output(char* path, int lflag);
int deallocate_node(struct List *list);

int push(struct Node *node);
struct Node* pop();
int queue_empty();
int queue_size();

int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct List list;
	struct List show_list;
	struct stat buf;
	int aflag = 0, lflag = 0;
	int index, c, i, j;
	struct info* output;
	char name[50];
	char path[100];
	char temp[100];
	char d_name[50];
	size_t size;
	opterr=0;
	list.pHead = NULL;


/*	printf("argc: %d, optind: %d\n", argc,optind);
	for(i=0;i<argc;i++)
		printf("%d: %s ",i, argv[i]);
	printf("\n");
*/
	while((c = getopt(argc, argv, "al"))!=-1)
	{
		switch(c)
		{
			case 'a':
				aflag = 1;
				break;
			case 'l':
				lflag = 1;
				break;
			case '?':
				printf("Unknown Option: %c\n",optopt);
				break;
		}
	}
	
/*	for(index=optind;index<argc;index++)
 *		printf("Non-option argument %s\n", argv[index]);
*/

//	printf("aflag = %d, lflag = %d \n", aflag, lflag);

	
	if(optind==argc)
	{
		//non-argument case
		dirp = opendir(".");

		while((dir=readdir(dirp))!=NULL){
			insert_node(&list, dir);
		}
		add_hidden_mark(&list);
		select_list(&list, aflag);			//-a option select file list so, come first
		create_output(path, lflag);		//current path and lflag
		closedir(dirp);
		//deallocate_node(&list);
	}
	else
	{
		for(index=optind; index<argc; index++)
		{
			getcwd(path, size);

			if(index!=optind)
				printf("\n");
			if(argv[index][0] == '/')		//절대경로  case
			{
				memset(temp, 0, sizeof(char)*100);
				i=strlen(argv[index]);
				while(1)
				{
					if(argv[index][i]=='/')
						break;
					i--;
				}
				for(j=0;j<i+1;j++)
					temp[j] = argv[index][j];
				i=j;
				j=0;
				for(;i<strlen(argv[index])+1;i++,j++)
					d_name[j] = argv[index][i];
				strcpy(path,argv[index]);
			}
			else		//상대경로
			{
				if(argv[index][0] == '.') //&& argv[index][1] == '.')	//current and parent dir
				{
					memset(temp, 0, sizeof(char)*100);
					strcat(path,"/");
					strcat(path, argv[index]);
					if(strlen(argv[index])==1 && argv[index][0]=='.')
						strcat(path,"/");
					else if(strlen(argv[index])==2 && argv[index][0]=='.' && argv[index][1]=='.')
						strcat(path,"/");

					i=strlen(path);

					while(1)
					{
						if(path[i]=='/')
							break;
						i--;
					}

					for(j=0;j<i+1;j++)
						temp[j] = path[j];

					memset(d_name,0,sizeof(char)*50);
					i=j;
					for(j=0;i<strlen(path);j++,i++)
						d_name[j] = path[i];

				}
				else	//current dir
				{
					//path+name
					memset(temp, 0, sizeof(char)*100);
					strcpy(temp,path);
					memset(d_name, 0, sizeof(char)*50);
					for(i=0;i<strlen(argv[index]);i++)
						d_name[i] = argv[index][i];
	
					strcat(path,"/");
					strcat(path,d_name);
				}	
			}

/*			printf("path: %s\n",path);
 *			printf("temp: %s\n",temp);
 *			printf("d_name: %s\n",d_name);
*/
			//File인지 Directory인지 검사
			if(stat(path,&buf)<0)
			{
				printf("ls: cannot access %s: No such file or directory\n",argv[index]);
			}

			if(S_ISDIR(buf.st_mode))	//directory
			{
				dirp = opendir(path);
				while((dir=readdir(dirp))!=NULL)
					insert_node(&list,dir);
			}
			else	//file
			{
				dirp = opendir(temp);
				while((dir=readdir(dirp))!=NULL)
				{
					if(strcmp(dir->d_name,d_name)==0)
						insert_node(&list, dir);
				}
				aflag = 1;
			}

			add_hidden_mark(&list);
			select_list(&list, aflag);
			create_output(temp, lflag);
			closedir(dirp);

			//deallocate_node(&list);
		}
	}



	return 0;
}

int insert_node(struct List *list, struct dirent *dir){
	struct Node *pCur = NULL, *pNext = NULL, *pPre =NULL;	
	struct Node *pNew = (struct Node*)malloc(sizeof(struct Node));
	
	pNew->d_name = (char*)malloc(sizeof(char)*strlen(dir->d_name)+1);
	memset(pNew->d_name, 0, sizeof(char)*strlen(dir->d_name)+1);

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

void select_list(struct List *list, int aflag)
{
	struct Node *pCur = list->pHead;

	if(aflag) //-a option case
	{
		while(pCur!=NULL)
		{
			push(pCur);
			pCur = pCur->pNext;
		}
	}
	else
	{
		while(pCur!=NULL)
		{
			if(pCur->hidden==0)
			{
				push(pCur);
			}
			pCur = pCur->pNext;
		}
	}
}

int create_output(char* path, int lflag)
{
	int i, size;
	struct stat buf;
	struct passwd *user_info;
	struct group *group_info;
	struct tm *tm_info;
	struct Node *pCur;
	size_t sizet;
	char *ptr;
	char a[10];
	char temp[100]={'\0'};
	char abs_path[150]={'\0'};

	pCur = pop();
	i=0;
	while(pCur!=NULL)
	{
		memset(temp, 0, sizeof(char)*100);
		memset(abs_path, 0, sizeof(char)*150);
		if(lflag==1)
		{
			sprintf(abs_path, "%s%s",path,pCur->d_name);
		if(stat(abs_path,&buf)<0){
			printf("ls: cannot access %s: No such file or directory\n",pCur->d_name);
			return -1;
		}

		if(S_ISREG(buf.st_mode))	//regular
			ptr = "-";
		else if(S_ISDIR(buf.st_mode))	//directory
			ptr = "d";
		else if(S_ISCHR(buf.st_mode))	//character special
			ptr = "c";
		else if(S_ISBLK(buf.st_mode))	//block special
			ptr = "b";
		else if(S_ISLNK(buf.st_mode))	//symbolic link
			ptr = "l";
		else if(S_ISSOCK(buf.st_mode))	//socket
			ptr = "s";
		else
			ptr = "** unknown mode **";

		sprintf(temp, "%s", ptr);

		sprintf(temp, "%s%c", temp, buf.st_mode&S_IRUSR ? 'r':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IWUSR ? 'w':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IXUSR ? 'x':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IRGRP ? 'r':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IWGRP ? 'w':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IXGRP ? 'x':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IROTH ? 'r':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IWOTH ? 'w':'-');
		sprintf(temp, "%s%c", temp, buf.st_mode&S_IXOTH ? 'x':'-');
	
		sprintf(temp,"%s %d", temp, (int)buf.st_nlink);
		
		user_info = getpwuid(buf.st_uid);
		group_info = getgrgid(buf.st_gid);
		tm_info = localtime(&buf.st_mtime);

		sprintf(temp, "%s %s %s", temp, user_info->pw_name, group_info->gr_name);
		sprintf(temp, "%s %5d", temp, (int)buf.st_size);
		
		sprintf(temp, "%s %s %2d %2d:%2d ", temp, mon_type[(tm_info->tm_mon)], tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min);
		}

		sprintf(temp, "%s%s", temp, pCur->d_name);
		
		//strcpy temp to output array

		printf("%s\n",temp);

		pCur = pop();
		i++;
	}

	return 0;
}

int deallocate_node(struct List *list)
{
	//de-allocate node in List

	return 0;
}



int push(struct Node *node)
{
	if(queue_empty())
	{
		queue.pHead = node;
		queue.pTail = node;
		return 0;
	}
	
	struct Node *pCur;

	pCur = queue.pTail;
	pCur->pQnext = node;
	queue.pTail = node;

	return 0;
}

struct Node* pop()
{
	struct Node *pop;
	
	if(queue_empty())	//fail pop from queue
		return NULL;

	pop = queue.pHead;
	queue.pHead = queue.pHead->pQnext;
	pop->pQnext=NULL;

	return pop;
}

int queue_empty()
{
	if(queue.pHead==NULL)
		return 1;

	return 0;
}

int queue_size()
{
	int i=0;
	struct Node* node;

	if(queue_empty())
		return i;
	node = queue.pHead;
	do
	{
		i++;
		node = node->pQnext;
	}while(node == queue.pTail);

	return i;
}
