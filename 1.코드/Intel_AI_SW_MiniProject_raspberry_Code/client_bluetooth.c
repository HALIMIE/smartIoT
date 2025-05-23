#include <arpa/inet.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 5

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[Default]";

typedef struct {
  int sockfd;
  int btfd;
  char sendid[NAME_SIZE];
} DEV_FD;

int main(int argc, char *argv[]) {
  DEV_FD dev_fd;
  struct sockaddr_in serv_addr;
  pthread_t snd_thread, rcv_thread;
  void *thread_return;
  int ret;
  struct sockaddr_rc addr = {0};
  char dest[18] = "98:DA:60:0D:AF:75";
  char msg[BUF_SIZE];

  if (argc != 4) {
    printf("Usage : %s <IP> <port> <name>\n", argv[0]);
    exit(1);
  }

  sprintf(name, "%s", argv[3]);

  dev_fd.sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (dev_fd.sockfd == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if (connect(dev_fd.sockfd, (struct sockaddr *)&serv_addr,
              sizeof(serv_addr)) == -1)
    error_handling("connect() error");

  sprintf(msg, "[%s:PASSWD]", name);
  write(dev_fd.sockfd, msg, strlen(msg));

  dev_fd.btfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if (dev_fd.btfd == -1) {
    perror("socket()");
    exit(1);
  }
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t)1;
  str2ba(dest, &addr.rc_bdaddr);
  ret = connect(dev_fd.btfd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1) {
    perror("connect()");
    exit(1);
  }

  pthread_create(&rcv_thread, NULL, recv_msg, (void *)&dev_fd);
  pthread_create(&snd_thread, NULL, send_msg, (void *)&dev_fd);
  pthread_join(snd_thread, &thread_return);
  close(dev_fd.sockfd);
  return 0;
}

void *send_msg(void *arg) {
  DEV_FD *dev_fd = (DEV_FD *)arg;
  int str_len;
  int ret;
  fd_set initset, newset;
  struct timeval tv;
  char name_msg[NAME_SIZE + BUF_SIZE + 2];
  char msg[BUF_SIZE];
  int total = 0;

  FD_ZERO(&initset);
  FD_SET(dev_fd->sockfd, &initset);
  FD_SET(dev_fd->btfd, &initset);

  while (1) {
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    newset = initset;
    ret = select(dev_fd->btfd + 1, &newset, NULL, NULL, &tv);
    if (FD_ISSET(dev_fd->btfd, &newset)) {
      ret = read(dev_fd->btfd, msg + total, BUF_SIZE - total);
      if (ret > 0) {
        total += ret;
      } else if (ret == 0) {
        dev_fd->sockfd = -1;
        return NULL;
      }

      if (msg[total - 1] == '\n') {
        msg[total] = 0;
        total = 0;
      } else
        continue;

      fputs("ARD:", stdout);
      fputs(msg, stdout);
      if (write(dev_fd->sockfd, msg, strlen(msg)) <= 0) {
        dev_fd->sockfd = -1;
        return NULL;
      }
    }
    if (ret == 0) {
      if (dev_fd->sockfd == -1) return NULL;
    }
  }
}

void *recv_msg(void *arg) {
  DEV_FD *dev_fd = (DEV_FD *)arg;
  int i;
  char *pToken;
  char *pArray[ARR_CNT] = {0};

  char name_msg[NAME_SIZE + BUF_SIZE + 1];
  int str_len;
  while (1) {
    memset(name_msg, 0x0, sizeof(name_msg));
    str_len = read(dev_fd->sockfd, name_msg, NAME_SIZE + BUF_SIZE);
    if (str_len <= 0) {
      dev_fd->sockfd = -1;
      return NULL;
    }
    name_msg[str_len] = 0;
    fputs("SRV:", stdout);
    fputs(name_msg, stdout);
    write(dev_fd->btfd, name_msg, strlen(name_msg));
  }
}

void error_handling(char *msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}
