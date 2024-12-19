#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

int main() {
    int fd = 
    open("/dev/rtc", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct rtc_time rtc_tm;
    if (ioctl(fd, RTC_RD_TIME, &rtc_tm) == -1) {
        perror("ioctl");
        return 1;
    }

    printf("RTC time: %d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1, rtc_tm.tm_mday,
           rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    close(fd);
    return 0;
}
