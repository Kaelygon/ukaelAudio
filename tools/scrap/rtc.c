



#define RTC_RD_TIME 0x80247009 // Read RTC time

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>

struct rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

int main() {
    int fd = open("/dev/rtc", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/rtc");
        return 1;
    }

    struct rtc_time rtc;
    if (ioctl(fd, RTC_RD_TIME, &rtc) < 0) {
        perror("RTC_RD_TIME ioctl failed");
        close(fd);
        return 1;
    }

    printf("RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc.tm_year + 1900, rtc.tm_mon + 1, rtc.tm_mday,
           rtc.tm_hour, rtc.tm_min, rtc.tm_sec);

    close(fd);
    return 0;
}