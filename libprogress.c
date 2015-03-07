#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


struct _progress {
	int fd;
	FILE *out;
	double period;
	uint64_t count;
	double start;
	double now;
	double last;
	off_t fd_size;
	off_t fd_pos;
};

struct _progress *P;

static double
getdtod(void)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0);
}

void
progressInit(int fd, FILE *out, time_t period)
{
	P = calloc(1, sizeof(*P));
	P->fd = fd;
	P->out = out;
	P->period = (double) period;
	P->start = P->last = getdtod();
	if (P->fd > -1) {
		struct stat sb;
		if (0 == fstat(P->fd, &sb)) {
			P->fd_size = sb.st_size;
		}
	}
}

void
progressTick(uint64_t count)
{
	double rate;
	P->now = getdtod();
	if (P->now - P->last < P->period)
		return;
	P->last = P->now;
	P->count  = count;
	rate = (double) P->count / (P->now - P->start);
	fprintf(P->out, "Progress: %u/sec", (unsigned int) rate);
	if (P->fd > -1) {
		double pct;
		double finished;
		unsigned int hh;
		unsigned int mm;
		unsigned int ss;
		P->fd_pos = lseek(P->fd, 0, SEEK_CUR);
		pct = (double) P->fd_pos / (double) P->fd_size;
		fprintf(P->out, ", %5.2f%% complete", 100.0 * pct);
		finished = (P->now - P->start) * (1.0 - pct) / pct;
		hh = (unsigned int) finished / 3600;
		mm = ((unsigned int) finished - hh * 3600) / 60;
		ss = ((unsigned int) finished - hh * 3600 - mm * 60);
		fprintf(P->out, ", finished in %u:%02u:%02u", hh, mm, ss);
	}
	fprintf(P->out, "\n");
}
