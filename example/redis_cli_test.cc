#include "redis_cli.h"

#include <stdio.h>
#include <errno.h>
#include "xdebug.h"

using namespace pink;

int main() {
  std::string str;
  int i = 5;

  printf ("\nTest Serialize\n");
  int ret = RedisCli::SerializeCommand(&str, "HSET %s %d", "key", i);
  printf ("   1. Serialize by va return %d, (%s)\n", ret, str.c_str());

  RedisCmdArgsType argv;
  argv.push_back("hset");
  argv.push_back("key");
  argv.push_back(std::to_string(5));

  ret = RedisCli::SerializeCommand(argv, &str);
  printf ("   2. Serialize by argv return %d, (%s)\n", ret, str.c_str());

  RedisCli *rcli = new RedisCli();
  rcli->set_connect_timeout(3000);

  printf ("  Connect with bind_ip(101.199.114.205)\n");
  Status s = rcli->Connect("127.0.0.1", 7221, "101.199.114.205");

  // Test connect timeout with a non-routable IP
  //Status s = rcli->Connect("10.255.255.1", 9824);

  printf(" RedisCli Connect return %s\n", s.ToString().c_str());
  if (!s.ok()) {
      printf ("Connect failed, %s\n", s.ToString().c_str());
      exit(-1);
  }

  ret = rcli->set_send_timeout(100);
  printf("set send timeout 100 ms, return %d\n", ret);

  ret = rcli->set_recv_timeout(100);
  printf("set recv timeout 100 ms, return %d\n", ret);

  /*
  char ch;
  scanf ("%c", &ch);
  */

  printf ("\nTest Send and Recv Ping\n");
  std::string ping = "*1\r\n$4\r\nping\r\n";
  for (int i = 0; i < 1; i++) {
    s = rcli->Send(&ping);
    printf("Send %d: %s\n", i, s.ToString().c_str());

    s = rcli->Recv(NULL);
    printf("Recv %d: return %s\n", i, s.ToString().c_str());
    if (rcli->argv_.size() > 0) {
      printf("  argv[0]  is (%s)\n", rcli->argv_[0].c_str());
    }
  }

  printf ("\nTest Send and Recv Mutli\n");
  RedisCli::SerializeCommand(&str, "MSET a 1 b 2 c 3 d 4");
  printf ("Send mset parse (%s)\n", str.c_str());
  s = rcli->Send(&str);
  printf ("Send mset return %s\n", s.ToString().c_str());

  s = rcli->Recv(NULL);
  printf("Recv mset return %s with %d elements\n", s.ToString().c_str());
  for (int i = 0; i < rcli->argv_.size(); i++) {
    printf("  argv[%d] = (%s)", i, rcli->argv_[i].c_str());
  }

  printf ("\n\nTest Mget case 1: send 1 time, and recv 1 time\n");
  RedisCli::SerializeCommand(&str, "MGET a  b  c  d ");
  printf ("Send mget parse (%s)\n", str.c_str());

  for (int si = 0; si < 2; si++) {
    s = rcli->Send(&str);
    printf ("Send mget case 1: i=%d, return %s\n", si, s.ToString().c_str());

    s = rcli->Recv(NULL);
    printf ("Recv mget case 1: i=%d, return %s with %d elements\n", si, s.ToString().c_str(), rcli->argv_.size());
    for (int i = 0; i < rcli->argv_.size(); i++) {
      printf("  argv[%d] = (%s)\n", i, rcli->argv_[i].c_str());
    }
  }

  printf ("\nTest Mget case 2: send 2 times, then recv 2 times\n");
  RedisCli::SerializeCommand(&str, "MGET a  b  c  d ");
  printf ("\nSend mget parse (%s)\n", str.c_str());

  for (int si = 0; si < 2; si++) {
    s = rcli->Send(&str);
    printf ("Send mget case 2: i=%d, return %s\n", si, s.ToString().c_str());
  }

  for (int si = 0; si < 2; si++) {
    s = rcli->Recv(NULL);
    printf ("Recv mget case 1: i=%d, return %s with %d elements\n", si, s.ToString().c_str(), rcli->argv_.size());
    for (int i = 0; i < rcli->argv_.size(); i++) {
      printf ("  argv[%d] = (%s)\n", i, rcli->argv_[i].c_str());
    }
  }

  char ch;
  scanf ("%c", &ch);

  return 0;
}
