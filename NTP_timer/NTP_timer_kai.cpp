#ifdef __GNUC__
#if !defined( WINVER ) || ( WINVER < 0x0501 )
#undef  WINVER
#define WINVER 0x0501
# endif
#if !defined( _WIN32_WINNT ) || ( _WIN32_WINNT < 0x0501 )
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif //__GNUC__
#include <winsock2.h>//�K��windows.h����
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>//in gcc
#include <errno.h>//in gcc
#include <stdint.h>
#include <stdbool.h>
#define diff_two_pram( first, second, width) (first < second)? first + width - second : first - second
#define diff_hour(first, second) diff_two_pram( first, second, 24)
#define diff_min(first, second) diff_two_pram( first, second, 60)
#define diff_sec diff_min
#ifndef EINVAL
#define EINVAL 22
#endif
#if !defined(_MSC_VER) || _MSC_VER < 1000
#ifndef WINSOCK_VERSION
#define WINSOCK_VERSION MAKEWORD(2,2)
#endif
typedef struct addrinfo ADDRINFOA;
typedef int errno_t;
errno_t localtime_s(struct tm *_Tm, const time_t* _Time){//localtime�֐��̕s��C���ŁAMSVC�̈����ƍ��킹�Ă�B
	if(NULL == _Tm || NULL == _Time) return EINVAL;
	struct tm *temp = localtime(_Time);
	if (NULL == temp) return EINVAL;
	*_Tm = *temp;//localtime�֐��͓�����static�ϐ��ւ̃|�C���^�[��Ԃ��̂ŁA������Ăяo������Ə����������Ă��܂�����R�s�[
	return 0;
}
errno_t gmtime_s(struct tm *_Tm, const time_t* _Time){//localtime�֐��̕s��C���ŁAMSVC�̈����ƍ��킹�Ă�B
	if (NULL == _Tm || NULL == _Time) return EINVAL;
	struct tm *temp = gmtime(_Time);
	if (NULL == temp) return EINVAL;
	*_Tm = *temp;//localtime�֐��͓�����static�ϐ��ւ̃|�C���^�[��Ԃ��̂ŁA������Ăяo������Ə����������Ă��܂�����R�s�[
	return 0;
}
#endif
#ifndef __GNUC__
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")//timeBeginPeriod,timeEndPeriod
#endif
typedef struct NTP_Packet {// NTP�p�P�b�g�\���� [RFC : 2030]
	int32_t Control_Word;
	int32_t root_delay;                    // ���[�g�x��
	int32_t root_dispersion;               // ���[�g���U
	int32_t reference_identifier;          // �ID
	int64_t reference_timestamp;       // ��^�C���X�^���v
	int64_t originate_timestamp;       // ��_�^�C���X�^���v
	int64_t receive_timestamp;         // ��M�^�C���X�^���v
	uint32_t transmit_timestamp_seconds;    // ���M�^�C���X�^���v
	uint32_t transmit_timestamp_fractions;  // ���M�^�C���X�^���v
}NTP_Packet;
int get_integer_num(const int max, const int min){
	//�@�\�F�W�����͂𐔎��ɕϊ�����B
	//�����F�߂�l�̍ő�l,�߂�l�̍ŏ��l
	//�߂�l�F���͂��������A�G���[����-1
	char s[100];
	long t;
	char *endptr;

	if (NULL == fgets(s, 100, stdin)) return -1;
	errno = 0;
	t = strtol(s, &endptr, 10);
	if (errno != 0 || *endptr != '\n' || t < min || max < t)
		return -1;
	return (int)t;
}
int get_integer_num_with_loop(const int max, const int min){
	int temp;
	while (-1 == (temp = get_integer_num(max, min)));
	return temp;
}
bool correct_npt_time(struct tm *_Tm, uint32_t delay_time){
	const uint32_t temp1 = delay_time / 60;
	_Tm->tm_sec += delay_time % 60;
	if (_Tm->tm_sec >= 60){
		_Tm->tm_sec -= 60;
		_Tm->tm_min++;
	}
	const uint32_t temp2 = temp1 / 60;
	_Tm->tm_min += temp1 % 60;
	if (_Tm->tm_min >= 60){
		_Tm->tm_min -= 60;
		_Tm->tm_hour++;
	}
	const uint32_t temp3 = temp2 / 24;
	_Tm->tm_hour += temp2 % 24;
	if (_Tm->tm_hour >= 24){
		_Tm->tm_hour -= 24;
		_Tm->tm_mday++;
	}
	const int temp = _Tm->tm_mday + temp3;
	if (temp > 28 || (1 != _Tm->tm_mon && temp >= 30))
		return false;
	_Tm->tm_mday = temp;
	return true;
}
bool SystemTimeToStruct_tm(struct tm* tm_date, SYSTEMTIME *sys_date){
	if (NULL == tm_date || NULL == sys_date) return false;
	tm_date->tm_hour = sys_date->wHour;
	tm_date->tm_min = sys_date->wMinute;
	tm_date->tm_mday = sys_date->wDay;
	tm_date->tm_mon = sys_date->wMonth - 1;
	tm_date->tm_sec = sys_date->wSecond;
	tm_date->tm_year = sys_date->wYear - 1900;
	tm_date->tm_wday = sys_date->wDayOfWeek;
	return true;
}
bool ToLocalStruct_tm(struct tm* _Tm){
	if (NULL == _Tm) return false;
	time_t temp = time(NULL);
	struct tm gm, local;
	if (0 != gmtime_s(&gm, &temp)) return false;
	if (0 != localtime_s(&local, &temp)) return false;
	_Tm->tm_mday += local.tm_mday - gm.tm_mday;
	_Tm->tm_hour += local.tm_hour - gm.tm_hour;
	_Tm->tm_min += local.tm_min - gm.tm_min;
	_Tm->tm_sec += local.tm_sec - gm.tm_sec;
	return true;
}
bool print_local_time(struct tm const* pnow, bool print_ymd){
	if (NULL == pnow) return false;
	static const char week[][7] = { "��", "��", "��", "��", "��", "��", "�y" };
	if (print_ymd){
		printf("������%2d�N%02d��%02d��(%s)", pnow->tm_year + 1900, pnow->tm_mon + 1, pnow->tm_mday, week[pnow->tm_wday]);
	}
	printf("%2d:%02d:%02d\n�ł��B\n", pnow->tm_hour, pnow->tm_min, pnow->tm_sec);
	return true;
}
bool Connect_Server_and_Convert(SYSTEMTIME *lpSystemTime, SOCKET sock, NTP_Packet* packet, struct timeval* lpwaitTime, fd_set* fds, uint32_t* delay_time){
	const clock_t connection_begin = clock();
	// ���M
	if (SOCKET_ERROR == send(sock, (const char *)(packet), sizeof(NTP_Packet), 0))
		return false;
	// ��M�҂�
	if (select(0, fds, NULL, NULL, (PTIMEVAL)lpwaitTime) <= 0)//@mavericktse:need explicit cast?
		return false;
	// ��M
	int recvLen = recv(sock, (char*)(packet), sizeof(NTP_Packet), 0);
	const clock_t connection_end = clock();
	if (SOCKET_ERROR == recvLen || 0 == recvLen || sizeof(NTP_Packet) != recvLen) return false;
	*delay_time = (uint32_t)((connection_end - connection_begin) / (clock_t)(CLOCKS_PER_SEC * 2));
	// �Œ菬���_���𕂓������_���֕ϊ�
	unsigned int f = ntohl(packet->transmit_timestamp_fractions);
	double frac = 0.0f, d = 0.5f;
	int i;
	for (i = sizeof(unsigned int) * 8 - 1; i >= 0; --i, d /= 2.0f)
		if (f & (1 << i)) frac += d;

	FILETIME ft;

	// 100�i�m�b�P�ʂ֕ϊ��A1900�N�`��1601�N�`�֕ϊ�
	*(uint64_t*)(&ft) =
		UInt32x32To64(ntohl(packet->transmit_timestamp_seconds), 10000000U) +
		(uint64_t)(frac * 10000000U) + 94354848000000000U;

	// FILETIME�\���̂���SYSTEMTIME�\���̂֕ϊ�
	if (0 == FileTimeToSystemTime(&ft, lpSystemTime)) return false;
	return true;
}
bool GetNtpTime2(SYSTEMTIME *lpSystemTime, ADDRINFOA* res_ai, struct timeval* lpwaitTime, uint32_t* delay_time){
	ADDRINFOA *res;
	SOCKET sock = INVALID_SOCKET;
	fd_set fds;
	for (res = res_ai; res != NULL; res = res->ai_next){
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);// �\�P�b�g�쐬
		if (INVALID_SOCKET != sock){
			if (0 == connect(sock, res->ai_addr, (int)(res->ai_addrlen))) break;// �ڑ�
			closesocket(sock);
			sock = INVALID_SOCKET;
		}
	}
	freeaddrinfo(res_ai);// addrinfo ��j��
	if (INVALID_SOCKET == sock) return false;

	// �t�@�C���f�B�X�N���v�^��������
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	// ���M�f�[�^�쐬
	NTP_Packet packet;//@mavericktse:removed {0}. Suppose to have a default constructor
	packet.Control_Word = htonl(0x0B000000);

	const bool errno_ = Connect_Server_and_Convert(lpSystemTime, sock, &packet, lpwaitTime, &fds, delay_time);
	// �ؒf
	shutdown(sock, SD_BOTH);
	closesocket(sock);

	return errno_;
}
bool GetNTPTime(struct tm* tm_date, const char* lpNtpServer, uint32_t timeout){
	if (NULL == tm_date || NULL == lpNtpServer) return false;
	SYSTEMTIME SystemTime;
	uint32_t delay_time = 0;
	WSADATA wsaData;// �\�P�b�g�������ϐ�
	// WinSock������
	if (0 != WSAStartup(WINSOCK_VERSION, &wsaData)) return false;
	ADDRINFOA ai, *res_ai;
	memset(&ai, 0, sizeof(ADDRINFOA));
	ai.ai_socktype = SOCK_DGRAM;
	ai.ai_family = PF_UNSPEC;
	bool errno_ = true;
	if (0 == getaddrinfo(lpNtpServer, "ntp", &ai, &res_ai)){// �A�h���X�ƃT�[�r�X��ϊ�
		struct timeval waitTime, *lpwaitTime = NULL;
		if (timeout != INFINITE){
			// �^�C���A�E�g��ݒ�
			waitTime.tv_sec = timeout / 1000;
			waitTime.tv_usec = (timeout % 1000) * 1000;
			lpwaitTime = &waitTime;
		}
		errno_ = GetNtpTime2(&SystemTime, res_ai, lpwaitTime, &delay_time);
	}
	WSACleanup();
	SystemTimeToStruct_tm(tm_date, &SystemTime);
	if(false == correct_npt_time(tm_date, delay_time)) return false;
	if(false == ToLocalStruct_tm(tm_date)) return false;
	return errno_;
}
bool calc_delay_betwin_npt_and_client(struct tm *delay, struct tm const*pnow, struct tm const* pnow_npt){
	if (NULL == delay || NULL == pnow || NULL == pnow_npt) return false;
	delay->tm_hour = pnow->tm_hour - pnow_npt->tm_hour;
	delay->tm_min = pnow->tm_min - pnow_npt->tm_min;
	delay->tm_sec = pnow->tm_sec - pnow_npt->tm_sec;
	return true;
}
bool proofreading_client_time(struct tm *pnow, struct tm const*delay){
	if (NULL == delay || NULL == pnow) return false;
	pnow->tm_sec -= delay->tm_sec;
	if (pnow->tm_sec < 0){
		pnow->tm_sec += 60;
		pnow->tm_min--;
	}
	pnow->tm_min -= delay->tm_min;
	if (pnow->tm_min < 0){
		pnow->tm_min += 60;
		pnow->tm_hour--;
	}
	pnow->tm_hour -= delay->tm_hour;
	if (pnow->tm_hour < 0){
		pnow->tm_hour += 24;
		pnow->tm_mday--;
		pnow->tm_wday--;
		if (pnow->tm_wday < 0) pnow->tm_wday += 7;
		if (pnow->tm_mday < 0){
			pnow->tm_mon--;
			if (pnow->tm_mon < 0){
				pnow->tm_mon += 12;
				pnow->tm_year--;
			}
			const uint32_t temp = (uint32_t)pnow->tm_mon;
			if (2 == temp)
				pnow->tm_mday += 28;
			else if (((1 == (temp & 1) && temp <= 7)) || (0 == (temp & 1) && temp >7))
				pnow->tm_mday += 31;
			else
				pnow->tm_mday += 30;
		}
	}
	return true;
}
int main(void){
	//int n, nO = -1; // �L�[�ԍ�
	//FILE *file;
	time_t now = time(NULL);//���sPC�̌��ݎ���
	struct tm pnow, pnow_npt, delay = { 0 };
	if (0 != localtime_s(&pnow, &now)) return -1;
	if (false == GetNTPTime(&pnow_npt, "ntp.nict.jp", 10000)) return -1;//npt�T�[�o�[�̌��ݎ���
	const clock_t loop_begin = clock();
	calc_delay_betwin_npt_and_client(&delay, &pnow, &pnow_npt);//npt�T�[�o�[�ƃN���C�A���g�Ƃ̎������v�Z�B
	print_local_time(&pnow, true);
	print_local_time(&pnow_npt, true);
	printf("�^�C�}�[���N�����܂��B\n"
		"����]�̎�������͂��AEnter�L�[����͂��Ă��������B\n");
	puts("����");
	const int timer_end_hour = get_integer_num_with_loop(24, 0);
	puts("����");
	const int timer_end_min = get_integer_num_with_loop(59, 0);
	puts("���b");
	const int timer_end_sec = get_integer_num_with_loop(59, 0);
	const clock_t duration = ((diff_hour(timer_end_hour, pnow_npt.tm_hour) * 24
		+ diff_min(timer_end_min, pnow_npt.tm_min)) * 60 + diff_sec(timer_end_sec, pnow_npt.tm_sec)) * CLOCKS_PER_SEC;
	system("cls");
	puts("�^�C�}�[����!!");
	timeBeginPeriod(1);// �^�C�}�[�̍ŏ����x��1msec�ɂ���
	while (clock() - loop_begin <= duration){
		clock_t turn_begin = clock();
		Beep(30000, (DWORD)CLOCKS_PER_SEC * 4 / 20);//����

		time_t curTime = time(NULL);
		struct tm pcurTime;
		if(0 != localtime_s(&pcurTime, &curTime)) return -1;
		proofreading_client_time(&pcurTime, &delay);//npt�T�[�o�[�ƃN���C�A���g�Ƃ̎�����␳
		print_local_time(&pcurTime, false);
		printf("�I�������F%d:%02d:%02d\n", timer_end_hour, timer_end_min, timer_end_sec);
		printf("�o�ߎ��ԁF%d�b\n", (clock() - loop_begin) / CLOCKS_PER_SEC);
		printf("�c�莞�ԁF%d�b\n", (duration - (clock() - loop_begin)) / CLOCKS_PER_SEC);
		Sleep((DWORD)CLOCKS_PER_SEC * 3 / 5);//�K����
		while ((clock() - turn_begin) < CLOCKS_PER_SEC);//��b���ƂɃ��[�v�����悤��
		//printf("��~����ɂ�Ctrl+C�L�[�������Ă��������B");
		system("cls");
	}
	Beep(3000, 3000);
	timeEndPeriod(1);// �^�C�}�[�̍ŏ����x��߂�
	/*file = fopen("pre-time.txt", "a+");
	fprintf(file, "h: %d m: %d", h, m);
	fscanf(file, "%d:%d", &h, &m);
	fclose(file);
	printf("�ߋ��̗����@1�@%d:%d\n", h, m);*/

	return 0;
}