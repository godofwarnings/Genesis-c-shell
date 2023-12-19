#ifndef _LIBS_
#define _LIBS_
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <termios.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>


//* My headers
#include "color.h"
#include "buffer_sizes.h"
#include "error_handling.h"
#include "jobs.h"
#include "execute.h"
#include "read_parse.h"
#include "signal.h"
#include "genesis-stdlib.h"
#include "shell.h"
#include "raw_mode.h"

#include "seek.h"
#include "pastevents.h"
#include "warp.h"
#include "proclore.h"
#include "peek.h"
#include "activities.h"
#include "ping.h"
#include "fg_bg.h"
#include "neonate.h"
#include "iman.h"


#endif