//
typedef long time_t;

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_wday;
	int tm_mday;
	int tm_mon;
	int tm_year;
};

struct tm *localtime(const time_t *t);
time_t time(time_t *t);
