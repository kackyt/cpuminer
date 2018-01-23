#include <stdio.h>
#include <pthread.h>
#include "stratum.h"

int
main()
{
  Stratum *st = Stratum::get_instance("stratum.misosi.ru",
                                     "16002",
                                     "kacky.test",
                                      "password",
                                      15);
  printf("%p\n", st);
  pthread_join(st->thread, NULL);
  return 0;
}
