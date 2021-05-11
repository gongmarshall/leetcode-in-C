#include <stdio.h>
#include <stdlib.h>
typedef struct bbtTM {
	int tm_sec; /* 秒 – 取值区间为[0,59] */
	int tm_min; /* 分 - 取值区间为[0,59] */
	int tm_hour; /* 时 - 取值区间为[0,23] */
	int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
	int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
	int tm_year; /* 年份，其值等于实际年份 */
} bbtTM_S;

const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const u_int32_t mon_yday[2][12] = {
	{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
	{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};


// 时间戳转年月日时分秒
void bbt_localtime(u_int32_t time, bbtTM_S *t)
{
	u_int32_t Pass4year;
	int hours_per_year;

	//取秒时间
	t->tm_sec=(int)(time % 60);
	time /= 60;
	//取分钟时间
	t->tm_min=(int)(time % 60);
	time /= 60;
	//取过去多少个四年，每四年有 1461*24 小时
	Pass4year = time / (1461L * 24L);
	//计算年份
	t->tm_year=(Pass4year << 2) + 1970;
	//四年中剩下的小时数
	time %= 1461L * 24L;
	//校正闰年影响的年份，计算一年中剩下的小时数
	while(1)
	{
		//一年的小时数
		hours_per_year = 365 * 24;
		//判断闰年，是闰年，一年则多24小时，即一天
		if ((t->tm_year & 3) == 0) hours_per_year += 24;

		if (time < hours_per_year) break;

		t->tm_year++;
		time -= hours_per_year;
	}
	//小时数
	t->tm_hour=(int)(time % 24);
	//一年中剩下的天数
	time /= 24;
	//假定为闰年
	time++;
	//校正闰年的误差，计算月份，日期
	if((t->tm_year & 3) == 0) {
		if (time > 60) {
			time--;
		} else {
			if (time == 60) {
				t->tm_mon = 1;
				t->tm_mday = 29;
				return ;
			}
		}
	}
	//计算月日
	for (t->tm_mon = 0; Days[t->tm_mon] < time;t->tm_mon++)
	{
		time -= Days[t->tm_mon];
	}

	t->tm_mday = (int)(time);

	return;
}


int bbtIsLeap(int year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

// 年月日时分秒转时间戳
u_int32_t bbt_mktime(bbtTM_S dt)
{
	u_int32_t ret;
	int i =0;
	// 以平年时间计算的秒数
	ret = (dt.tm_year - 1970) * 365 * 24 * 3600;
	ret += (mon_yday[bbtIsLeap(dt.tm_year)][dt.tm_mon] + dt.tm_mday - 1) * 24 * 3600;
	ret += dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
	// 加上闰年的秒数
	for(i=1970; i < dt.tm_year; i++)
	{
		if(bbtIsLeap(i)) {
			ret += 24 * 3600;
		}
	}
	if (ret > 4107715199) { //2100-02-29 23:59:59
		ret += 24 * 3600;
	}
	return(ret);
}

int main()
{
	bbtTM_S date;
	unsigned int c_time = 1620697371;
	bbt_localtime(c_time, &date);
	printf("%d-%d-%d %d:%d:%d \n", date.tm_year, date.tm_mon+1, date.tm_mday, date.tm_hour+8, date.tm_min, date.tm_sec);
	return 0;
}
