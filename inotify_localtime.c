#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define LOCALTIMEPATH "/etc/localtime"
#define LOCALTIMEZONE "/usr/share/zoneinfo/Asia/Shanghai"
#define DIFILEPATH "/home/rocktech/diff.txt"

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = 0;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
} 

int watch_inotify_events(int fd)
{
	int ret;
	unsigned long tmp;
	char event_buf[512];
	int event_pos = 0;
	int event_size = 0;
	struct inotify_event *event;

	while(1) {
		ret = read(fd, event_buf, sizeof(event_buf));
		if(ret < (int)sizeof(struct inotify_event))
		{
			printf("counld not get event!\n");
			return -1;
		}

		while(ret >= (int)sizeof(struct inotify_event) )
		{
			event = (struct inotify_event*)(event_buf + event_pos);
			if(event->len)
			{
				if(strstr(event->name, "localtime") != NULL) {
					if(event->mask & IN_DELETE)
					{
						printf("delete file: %s\n",event->name);
						sleep(1);
						if (access(LOCALTIMEPATH,F_OK)==0)
						{
							system("diff /etc/localtime /usr/share/zoneinfo/Asia/Shanghai > /home/rocktech/diff.txt");
							tmp = get_file_size(DIFILEPATH);
							if (tmp > 0)
								system("sudo ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime");
						} else {
							system("sudo ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime");
						}
					}
				}
			}
			event_size = sizeof(struct inotify_event) + event->len;
			ret -= event_size;
			event_pos += event_size;
		}
		ret=0;
		event_buf[512]=0;
		event_pos=0;
		event_size=0;
	}

	return 0;  
}

int main(int argc, char** argv)
{
	int ret;
	int InotifyFd;

	if(argc != 2)  
	{
		printf("Usage: %s <dir>\n", argv[0]);
		return -1;
	}

	InotifyFd = inotify_init();
	if(InotifyFd == -1)
	{  
		printf("inotify_init error!\n");
		return -1;
	}

	ret = inotify_add_watch(InotifyFd, argv[1], IN_DELETE);
	watch_inotify_events(InotifyFd);

	if(inotify_rm_watch(InotifyFd, ret) == -1)
	{
		printf("notify_rm_watch error!\n");
		return -1;
	}

	close(InotifyFd);  
	return 0;  
}  
