/*
  Based on yabai.c & SocketClient.c
  https://github.com/koekeishiya/yabai/blob/master/src/yabai.c
  https://github.com/xiamaz/YabaiIndicator/blob/main/YabaiIndicator/SocketClient.c
*/

#include "yabai_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define MAXLEN          512
#define SOCKET_PATH_FMT "/tmp/yabai_%s.socket"
#define FAILURE_MESSAGE "\x07"

static inline int socket_connect(int *sockfd, char *socket_path)
{
  struct sockaddr_un socket_address;
  socket_address.sun_family = AF_UNIX;

  if (*sockfd == -1)
    return -1;

  snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
  return connect(*sockfd, (struct sockaddr *)&socket_address, sizeof(socket_address)) != -1;
}

static inline void socket_close(int sockfd)
{
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
}

static char *prepare_message(int argc, char **argv, int *message_length)
{
  *message_length = argc;
  size_t argl[argc];

  for (int i = 1; i < argc; ++i) {
    argl[i] = strlen(argv[i]);
    *message_length += argl[i];
  }

  char *message = malloc(sizeof(int) + *message_length);
  char *temp = sizeof(int) + message;

  memcpy(message, message_length, sizeof(int));

  for (int i = 1; i < argc; ++i) {
    strcpy(temp, argv[i]);
    temp += argl[i] + 1;
  }
  *temp++ = '\0';

  return message;
}

int send_query(int argc, char **argv, char **response)
{
  if (argc <= 1 || !response) {
    return EXIT_FAILURE;
  }

  *response = calloc(BUFSIZ, sizeof(char));
  if (!*response) {
    return EXIT_FAILURE;
  }

  char *user = getenv("USER");
  if (!user) {
    snprintf(*response, BUFSIZ, "yabai-msg: 'env USER' not set! abort..\n");
    return EXIT_FAILURE;
  }

  int message_length;
  char *message = prepare_message(argc, argv, &message_length);

  int sockfd;
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    snprintf(*response, BUFSIZ, "yabai-msg: failed to open socket..\n");
    return EXIT_FAILURE;
  }

  char socket_file[MAXLEN];
  snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

  if (!socket_connect(&sockfd, socket_file)) {
    snprintf(*response, BUFSIZ, "yabai-msg: failed to connect to socket..\n");
    return EXIT_FAILURE;
  }

  if (send(sockfd, message, sizeof(int) + message_length, 0) == -1) {
    snprintf(*response, BUFSIZ, "yabai-msg: failed to send data..\n");
    return EXIT_FAILURE;
  }

  shutdown(sockfd, SHUT_WR);
  free(message);

  int result = EXIT_SUCCESS;
  size_t bytes_read = 0;
  size_t total_bytes_read = 0;
  size_t result_size = BUFSIZ;
  char rsp[BUFSIZ];

  while ((bytes_read = read(sockfd, rsp, sizeof(rsp) - 1)) > 0) {
    rsp[bytes_read] = '\0';
    if (total_bytes_read + bytes_read >= result_size) {
      result_size *= 2;
      char *temp = realloc(*response, result_size);
      if (!temp) {
        free(*response);
        *response = NULL;
        return EXIT_FAILURE;
      }
      *response = temp;
    }
    if (rsp[0] == FAILURE_MESSAGE[0]) {
      result = EXIT_FAILURE;
      memcpy((*response) + total_bytes_read, rsp + 1, bytes_read);
    }
    else {
      memcpy((*response) + total_bytes_read, rsp, bytes_read + 1);
    }
    total_bytes_read += bytes_read;
  }
  socket_close(sockfd);
  return result;
}

int send_cmd(int argc, char **argv)
{
  if (argc <= 1) {
    return EXIT_FAILURE;
  }

  char *user = getenv("USER");
  if (!user) {
    return EXIT_FAILURE;
  }

  int message_length;
  char *message = prepare_message(argc, argv, &message_length);

  int sockfd;
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    free(message);
    return EXIT_FAILURE;
  }

  char socket_file[MAXLEN];
  snprintf(socket_file, sizeof(socket_file), SOCKET_PATH_FMT, user);

  if (!socket_connect(&sockfd, socket_file)) {
    free(message);
    return EXIT_FAILURE;
  }

  if (send(sockfd, message, sizeof(int) + message_length, 0) == -1) {
    free(message);
    return EXIT_FAILURE;
  }

  free(message);
  socket_close(sockfd);
  return EXIT_SUCCESS;
}
