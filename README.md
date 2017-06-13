# ls_Webserver
> Linux Ubuntu의 ls 기능을 제공하는 Web Server를 구현하는 프로젝트이다. 순차적으로 먼저 여러 옵션을 사용할 수 있는 ls 프로그램을 구현하고 이를 Web Server에서 서비스 하도록 한다.

## ls기능
1. **ls [filename or directory]**
	- 기본 ls 기능으로 해당 디렉토리의 hidden 파일을 제외한 파일 목록을 출력한다.
	- 선택적으로 파일 혹은 디렉토리 인자를 받을 수 있다.
2. **ls -a [filename or directory]**
	- -a 옵션으로 해당 디렉토리의 hidden 파일을 포함한 모든 파일 목록을 출력한다.
	- 선택적으로 파일 혹은 디렉토리 인자를 받을 수 있다.
3. **ls -l [filename or directory]**
	- -l 옵션으로 해당 디렉토리의 hidden 파일을 제외한 파일의 Permission, 소유자, 그룹, 크기, 날짜 등의 정보를 출력한다.
	- 선택적으로 파일 혹은 디렉토리 인자를 받을 수 있다.
4. **ls -al [filename or directory]**
	- -a, -l 옵션을 함께 적용해서 해당 디렉토리의 hidden 파일을 포함한 모든 파일의 Permission, 소유자, 그룹, 크기, 날짜 등의 정보를 출력한다.
5. **ls wild card matching
	- wild card matching ('*', '?', '[seq]')
		- '*': 길이에 상관없이 문자열 matching
		- '?': 하나의 character에 대해서 matching
		- '[seq]': seq내의 character에 대해서 matching
	- Passing arguments with quotes ( eg. ./ls '*')
	- Implements -h, -s, -S option

## Web Server

