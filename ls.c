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
	char d_name[100];
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

#define _MAX_PATH 300

struct Queue queue;
char *mon_type[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int insert_node(struct List *list, struct dirent *dir);
void add_hidden_mark(struct List *list);
void select_list(struct List *list, int aflag);
int create_output(FILE* fp, char* path, int lflag);
int deallocated_node(struct List *list);
void get_argument_path(char* argv, char* path, char* temp, char* d_name, int index);
void create_path(char* path);
void del_front_path(char* path);

int push(struct Node *node);
struct Node* pop();
int queue_empty();

int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct List list;
	struct stat buf;
	int aflag = 0, lflag = 0;
	int index, c, s_dir=0, i;
	char path[150];
	char temp[150];
	char d_name[50];
	char cmd[100]={'\0'};
	FILE *fp=fopen("html_ls.html","w");
	if(fp==0){
		printf("File Open ERROR!\n");
		return -1;
	}

	opterr=0;
	list.pHead = NULL;
	
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

	for(i=0;i<argc;i++)
	{
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}
	getcwd(path,_MAX_PATH);

	fprintf(fp,"<html>\n");
	fprintf(fp,"<head>\n");
	fprintf(fp,"<title>%s</title>\n",path);
	fprintf(fp,"<b><h3>%s</h3></b>\n",cmd);
	fprintf(fp,"<style> table, th, td{border: 1px solid black; border-collapse:collapse;} th{text-align:center;}</style>\n");
	fprintf(fp,"</head>\n");
	fprintf(fp,"<body>\n");

	if(optind==argc)
	{
		//non-argument case
		printf("Directory path: %s\n",path);
		fprintf(fp,"<h4>Directory path: %s</h4>",path);
		dirp = opendir(".");
		while((dir=readdir(dirp))!=NULL){
			insert_node(&list, dir);
		}
		add_hidden_mark(&list);
		select_list(&list, aflag);			//-a option select file list so, come first
		
		create_output(fp, path, lflag);		//current path and lflag
		closedir(dirp);
		deallocated_node(&list);
	}
	else
	{
		for(index=optind; index<argc; index++)
		{
			memset(path,0,sizeof(char)*150);
			memset(temp,0,sizeof(char)*150);
			memset(d_name,0,sizeof(char)*50);
	
			get_argument_path(argv[index],path, temp, d_name, index);

			printf("Directory path: %s\n",path);
			fprintf(fp,"<h4>Directory path: %s</h4>",path);
//			printf("temp: %s\n",temp);
//			printf("d_name: %s\n",d_name);

			//File인지 Directory인지 검사
			if(stat(path,&buf)<0)
			{
				printf("ls: cannot access %s: No such file or directory\n",argv[index]);
				fprintf(fp,"<p>ls: cannot access %s: No such file or directory\n",argv[index]);
			}
			else
			{
				if(S_ISDIR(buf.st_mode))	//directory
				{
					dirp = opendir(path);
					while((dir=readdir(dirp))!=NULL)
						insert_node(&list,dir);
					s_dir = 1;
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
					s_dir = 0;
				}

				add_hidden_mark(&list);
				select_list(&list, aflag);
				create_output(fp,s_dir==1 ? path:temp, lflag);
				closedir(dirp);
				deallocated_node(&list);
			}
		}
	}
	
	fprintf(fp,"</body>\n");
	fprintf(fp,"</html>\n");

	return 0;
}

void create_path(char* path)
{
	char *e;
	char temp[150]={'\0'};

	e = strtok(path,"/");
	if(strcmp(e,"..")==0 || strcmp(e,".")==0)
		temp[0]='/';
	else
		sprintf(temp, "%s%s", "/", e);

	while(e = strtok(NULL,"/"))
	{
		if(strcmp(e,"..")==0)
			del_front_path(temp);
		else if(!strcmp(e,".")==0)
			sprintf(temp,"%s%s%s",temp,"/",e);
	}
	
	memset(path,0,sizeof(char)*150);
	strcpy(path,temp);
}

void del_front_path(char* path)
{
	int i;

	i=strlen(path);
	while(i>=0)
	{
		if(path[i]=='/')
			break;
		path[i]='\0';
		i--;
	}
	path[i]='\0';
}

void get_argument_path(char* argv, char* path, char* temp, char* d_name, int index)
{
	int i,j;
	struct stat buf;
	size_t size;
	int bufsize;
	char* ptr;

	ptr = getcwd(path, _MAX_PATH);

	if(index!=optind)
		printf("\n");
	if(argv[0] == '/')		//절대경로  case
	{
		memset(temp, 0, sizeof(char)*150);
		i=strlen(argv);
		while(1)
		{
			if(argv[i]=='/')
				break;
			i--;
		}
		for(j=0;j<i;j++)
			temp[j] = argv[j];
		i=++j;
		j=0;
		for(;i<strlen(argv)+1;i++,j++)
			d_name[j] = argv[i];
		strcpy(path,argv);
	}
	else		//상대경로
	{
		if(argv[0] == '.') //current and parent dir
		{
			memset(temp, 0, sizeof(char)*150);
			strcat(path,"/");
			strcat(path, argv);

			if(strlen(argv)==1 && argv[0]=='.')
				strcat(path,"/");
			else if(strlen(argv)==2 && argv[0]=='.' && argv[1]=='.')
				strcat(path,"/");

			create_path(path);
	
			i=strlen(path);
			while(i>=0)
			{
				if(path[i]=='/')
					break;
				i--;
			}

			for(j=0;j<i;j++)
				temp[j] = path[j];
		
			memset(d_name,0,sizeof(char)*50);
			i=++j;
			for(j=0;i<strlen(path);j++,i++)
				d_name[j] = path[i];

		}
		else	//current dir
		{
			//path+name
			memset(temp, 0, sizeof(char)*150);
			strcpy(temp,path);
			memset(d_name, 0, sizeof(char)*50);
			for(i=0;i<strlen(argv);i++)
				d_name[i] = argv[i];

			strcat(path,"/");
			strcat(path,d_name);
		}	
	}
}

int insert_node(struct List *list, struct dirent *dir){
	struct Node *pCur = NULL, *pNext = NULL, *pPre =NULL;	
	struct Node *pNew = (struct Node*)malloc(sizeof(struct Node));
	
	memset(pNew->d_name, 0, sizeof(char)*100);

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

int create_output(FILE* fp, char* path, int lflag)
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
	char arr[100]={'\0'};
	char abs_path[150]={'\0'};

	fprintf(fp,"<hr>\n");
	fprintf(fp,"<table style='width:80%%'>\n");

	if(lflag==1)
		fprintf(fp,"<tr>\n<th>Name</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>LastModified</th>\n</tr>\n");
	else
		fprintf(fp,"<tr><th>Name</th><tr>\n");

	pCur = pop();
	i=0;
	while(pCur!=NULL)
	{
		memset(temp, 0, sizeof(char)*100);
		
		if(strcmp("html_ls.html",pCur->d_name)==0)
		{
			pCur = pop();
			continue ;
		}

		fprintf(fp,"<tr>\n");
		fprintf(fp,"<td><a href='./ls /'>%s</a></td>\n",pCur->d_name);

		if(lflag==1)
		{
			memset(arr, 0, sizeof(char)*100);
					memset(abs_path, 0, sizeof(char)*150);
		
			sprintf(abs_path, "%s%s%s",path,"/",pCur->d_name);
			if(stat(abs_path,&buf)<0)
			{
				printf("ls: cannot access %s: No such file or directory\n",pCur->d_name);
				fprintf(fp,"<p>ls: cannot access %s: No such file or directory\n",pCur->d_name);
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

			//sprintf(temp, "%s", ptr);
				
			sprintf(arr, "%s", ptr);
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IRUSR ? 'r':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IWUSR ? 'w':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IXUSR ? 'x':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IRGRP ? 'r':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IWGRP ? 'w':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IXGRP ? 'x':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IROTH ? 'r':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IWOTH ? 'w':'-');
			sprintf(arr, "%s%c", arr, buf.st_mode&S_IXOTH ? 'x':'-');
	
			fprintf(fp,"<td align='center'>%s</td>\n",arr);
			
			sprintf(temp,"%s %d", arr, (int)buf.st_nlink);
			fprintf(fp,"<td align='right'>%d</td>\n",(int)buf.st_nlink);

			user_info = getpwuid(buf.st_uid);
			group_info = getgrgid(buf.st_gid);
			tm_info = localtime(&buf.st_mtime);

			sprintf(temp, "%s %s %s", temp, user_info->pw_name, group_info->gr_name);
			fprintf(fp,"<td align='center'>%s</td>\n",user_info->pw_name);
			fprintf(fp,"<td align='center'>%s</td>\n",group_info->gr_name);

			sprintf(temp, "%s %5d", temp, (int)buf.st_size);
			fprintf(fp,"<td align='right'>%d</td>\n",(int)buf.st_size);
		
			memset(arr, 0, sizeof(char)*100);
			sprintf(arr, "%s %2d %2d:%2d", mon_type[(tm_info->tm_mon)], tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min);
			fprintf(fp,"<td align='center'>%s</td>\n",arr);

			sprintf(temp, "%s %s ", temp, arr);
		}
		

		sprintf(temp, "%s%s", temp, pCur->d_name);
	
		fprintf(fp,"</tr>\n");

		printf("%s\n",temp);

		pCur = pop();
		i++;
	}

	fprintf(fp,"</table>\n");

	return 0;
}

int deallocated_node(struct List *list)
{
	//de-allocate node in List
	struct Node* pCur = list->pHead;

	while(list->pHead!=NULL)
	{
		list->pHead = list->pHead->pNext;
		pCur->pNext = NULL;
		free(pCur);
		pCur = list->pHead;
	}
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

