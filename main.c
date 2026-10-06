// === include ===
#include <stdio.h>

#include <windows.h>
#include <time.h>
#include <tlhelp32.h>

#include "common/types.h"
#include "common/functions.h"

// === defines ===
#define	TRUE	1
#define	FALSE	0

#define	DEF_COLOR_WELCOME	0x0dU
#define	DEF_COLOR_DEBUG		0x0bU
#define	DEF_COLOR_TIMESTAMP	0x0fU
#define	DEF_COLOR_REGULAR	0x07U
#define	DEF_COLOR_ERROR		0x0cU
#define	DEF_COLOR_PROC_RUN	0x02U
#define	DEF_COLOR_PROC_END	0x06U
#define	DEF_COLOR_PROC_TERM	0x04U

#define	DEF_CONSOLE_TITLE	"WerFault catcher"
#define	DEF_WELCOME_TEXT	"WerFault catcher for Windows Vista\n\n"

#define	DEF_SS_IN_DD	86400U
#define	DEF_SS_IN_HH	3600U
#define	DEF_SS_IN_MM	60U

#define	DEF_SLEEP_MIN	50U
#define	DEF_SLEEP_REG	100U
#define	DEF_SLEEP_MAX	10000U


int main(int argument_qnnt, char *argument_string[]) {
/***************/
/* declaration */
/* */	TYPE_BOOL		mode_DEBUG = 0;
/* */	TYPE_UINT		sleep_value = DEF_SLEEP_REG;
/* */	size_t			idx = 0;
/* */
/* */	OSVERSIONINFO	Win_version;
/* */
/* */	HANDLE			process_tree_snapshot;
/* */	HANDLE			process_tree_snapshot2;
/* */	PROCESSENTRY32	struct_process;
/* */	PROCESSENTRY32	struct_process2;
/* */
/* */	const TYPE_BYTE	*CONST_TARGET1_TITLE;
/* */	const TYPE_BYTE	*CONST_TARGET1_NAME;
/* */	const TYPE_BYTE	*CONST_TARGET2_NAME;
/* */	TYPE_UINT		CONST_Win_version_major;
/* */
/* */	TYPE_BOOL		TARGET2_running = 0;
/* */	time_t			TARGET2_running_timestamp = 0;
/* */	time_t			TARGET2_terminate_timestamp = 0;
/* */	time_t			TARGET2_uptime = 0;
/* */	TYPE_UDINT		TARGET2_uptime_DD = 0;
/* */	TYPE_UDINT		TARGET2_uptime_HH = 0;
/* */	TYPE_UDINT		TARGET2_uptime_MM = 0;
/* */	TYPE_UDINT		TARGET2_uptime_SS = 0;
/* */
/* */	TYPE_BYTE	fct_fprint_log_result = 0;
/* */	TYPE_BYTE	log_text_size[256] = {0};
/* END declaration */
/*******************/

// === code ===
//	задаем заголовок окна и вступительный текст
SetConsoleTitleA(DEF_CONSOLE_TITLE);
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_WELCOME);
printf(DEF_WELCOME_TEXT);
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

//	если переданы аргументы (нулевой=название программы)
if (argument_qnnt > 1) {
	//	пробегаемся по аргументам от 1го до количества переданных
	for (idx = 1; idx < argument_qnnt; idx = idx + 1) {
		// если совпадает с переданным кодовым словом
		if (_stricmp(argument_string[idx], "/debug") == 0) {
			mode_DEBUG = 1;
		// если совпадает с переданным кодовым словом
		} else if (_stricmp(argument_string[idx], "/sleep") == 0) {
			//	проверяем наличие следующего аргумента (должен содержать число)
			if (idx + 1 < argument_qnnt) {
				//	вызываем функцию обработки аргумента
				fct_parse_numeric_argument((const TYPE_BYTE *)argument_string[idx + 1], DEF_SLEEP_MIN, DEF_SLEEP_MAX, &sleep_value);

				//	перескакиваем через числовой аргумент
				idx = idx + 1;
			} else {
				printf("ERROR: no number value after /sleep\n");
			}
		} else {
			printf("WARNING: unknown argument '%s'\n", argument_string[idx]);
		}
	}
}

if (mode_DEBUG) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_DEBUG);
	printf("DEBUG mode: ON\n\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

	CONST_TARGET1_TITLE = "HxD";
	CONST_TARGET1_NAME = "HxD64.exe";
	CONST_TARGET2_NAME = "calc.exe";
	CONST_Win_version_major = 5;	// XP=5.1 / Vista=6.0 / 7=6.1 / 8=6.2 / 8.1=6.3
} else {
	CONST_TARGET1_TITLE = "prog title";
	CONST_TARGET1_NAME = "WerFault.exe";
	CONST_TARGET2_NAME = "prog.exe";
	CONST_Win_version_major = 6;	// XP=5.1 / Vista=6.0 / 7=6.1 / 8=6.2 / 8.1=6.3
}

//	получаем версию ОС
Win_version.dwOSVersionInfoSize = sizeof(Win_version);

if (GetVersionEx(&Win_version)) {
	//	если версия ОС менее требуемой
	if (Win_version.dwMajorVersion < CONST_Win_version_major) {
		//	выводим ошибку и закрываемся
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_ERROR);
		printf("ERROR: Windows Vista or newer is required\n\n");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
		printf("Press ENTER to EXIT...");
		getchar();
		return 2;
	} else {
		while (1) {
			//	если цель не запущена
			if (!TARGET2_running) {
				//	получаем снимок процессов
				process_tree_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				//	если не удается получить снимок процессов
				if (process_tree_snapshot == INVALID_HANDLE_VALUE) {
					//	выводим ошибку
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_ERROR);
					printf("ERROR: CreateToolhelp32Snapshot failed\n");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
				} else {
					//	заносим в структуру процесса первый элемент списка процессов
					struct_process.dwSize = sizeof(struct_process);
					if (Process32First(process_tree_snapshot, &struct_process)) {
						do {
							//	ищем в структуре совпадение имени exe с целевым
							if (_stricmp(struct_process.szExeFile, CONST_TARGET2_NAME) == 0) {
								//	если совпало, ставим флаг активности цели
								TARGET2_running = TRUE;
								//	получаем метку времени
								time(&TARGET2_running_timestamp);

								//	затираем пробелами 50 символов с начала строки, выводим локальное время и уведомление об активности цели
								printf("\r%-50s\r", " ");
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_TIMESTAMP);
								fct_print_local_time();
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
								printf("Process %s, PID: %lu ", struct_process.szExeFile, struct_process.th32ProcessID);
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_PROC_RUN);
								printf("is running\n\n");
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

								snprintf(log_text_size, sizeof(log_text_size), "Process %s, PID: %lu is running\n", struct_process.szExeFile, struct_process.th32ProcessID);
								fct_fprint_log_result = fct_fprint_log(log_text_size);
								if (fct_fprint_log_result != 0) {
									printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

									//	закрываем открытый handle списка процессов
									CloseHandle(process_tree_snapshot);
									return 5;
								}

								//	завершаем перебор
								break;
							}
						} while (Process32Next(process_tree_snapshot, &struct_process));	//	перебираем элементы списка процессов
					}
					//	закрываем открытый handle списка процессов
					CloseHandle(process_tree_snapshot);
				}
			}

			fct_throbber();

			//	ожидаем появления целевого окна
			HWND	target_HWND = FindWindowA(NULL, CONST_TARGET1_TITLE);

			//	если окно появилось
			if (target_HWND != NULL) {
				//	затираем пробелами 50 символов с начала строки, выводим локальное время
				printf("\r%-50s\r", " ");
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_TIMESTAMP);
				fct_print_local_time();
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

				// определяем PID по "заголовку" окна
				DWORD	target_PID = 0;
				GetWindowThreadProcessId(target_HWND, &target_PID);
				printf("HWND: %p => PID: %lu\n", target_HWND, target_PID);

				//	получаем снимок процессов
				process_tree_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

				if (process_tree_snapshot == INVALID_HANDLE_VALUE) {
					// если не удалось получить снимок процессов выводим ошибку
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_ERROR);
					printf("ERROR: CreateToolhelp32Snapshot failed\n");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
				} else {
					//	заносим в структуру процесса первый элемент списка процессов
					struct_process.dwSize = sizeof(struct_process);
					if (Process32First(process_tree_snapshot, &struct_process)) {
						do {
							//	ищем в структуре совпадение PID целевого процесса
							if (struct_process.th32ProcessID == target_PID) {
								printf("PID: %lu => process: %s\n", struct_process.th32ProcessID, struct_process.szExeFile);
								//	проверяем, что имя exe в структуре процесса совпадает с искомым
								if (_stricmp(struct_process.szExeFile, CONST_TARGET1_NAME) == 0) {
									// отправляем запрос на завершение
									SendMessageA(target_HWND, WM_CLOSE, 0, 0);
									printf("Process %s ", struct_process.szExeFile);
									SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_PROC_END);
									printf("ended\n\n");
									SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

									snprintf(log_text_size, sizeof(log_text_size), "Process %s ended\n", struct_process.szExeFile);
									fct_fprint_log_result = fct_fprint_log(log_text_size);
									if (fct_fprint_log_result != 0) {
										printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

										//	закрываем открытый handle списка процессов
										CloseHandle(process_tree_snapshot);
										return 5;
									}

									while (1) {
										//	получаем ещё 1 снимок процессов
										process_tree_snapshot2 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
										if (process_tree_snapshot2 == INVALID_HANDLE_VALUE) {
											// если не удалось получить снимок процессов выводим ошибку
											SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_ERROR);
											printf("ERROR: CreateToolhelp32Snapshot 2 failed\n");
											SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
										} else {
											TYPE_BOOL	target2_active = 0;
											//	заносим в структуру процесса первый элемент списка процессов
											struct_process2.dwSize = sizeof(struct_process2);
											if (Process32First(process_tree_snapshot2, &struct_process2)) {
												do {
													//	проверяем, что имя exe в структуре процесса совпадает с искомым
													if (_stricmp(struct_process2.szExeFile, CONST_TARGET2_NAME) == 0) {
														printf("Process %s: ", struct_process2.szExeFile);
														SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_TIMESTAMP);
														printf("active\n");
														SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);
														target2_active = 1;
														break;
													}
												} while (Process32Next(process_tree_snapshot2, &struct_process2));
											}

											if (!target2_active) {
												TARGET2_running = FALSE;
												time(&TARGET2_terminate_timestamp);
												TARGET2_uptime = TARGET2_terminate_timestamp - TARGET2_running_timestamp;
												TARGET2_uptime_DD = (TYPE_UDINT)TARGET2_uptime / DEF_SS_IN_DD;
												TARGET2_uptime_HH = ((TYPE_UDINT)TARGET2_uptime % DEF_SS_IN_DD) / DEF_SS_IN_HH;
												TARGET2_uptime_MM = ((TYPE_UDINT)TARGET2_uptime % DEF_SS_IN_HH) / DEF_SS_IN_MM;
												TARGET2_uptime_SS = (TYPE_UDINT)TARGET2_uptime % DEF_SS_IN_MM;
												if (TARGET2_uptime_DD > 0) {
													printf("Process %s uptime: ", CONST_TARGET2_NAME);
													SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_TIMESTAMP);
													printf("%u day(s) %u:%02u:%02u\n\n", TARGET2_uptime_DD, TARGET2_uptime_HH, TARGET2_uptime_MM, TARGET2_uptime_SS);
													SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

													snprintf(log_text_size, sizeof(log_text_size), "Process %s uptime: %u day(s) %u:%02u:%02u\n", CONST_TARGET2_NAME, TARGET2_uptime_DD, TARGET2_uptime_HH, TARGET2_uptime_MM, TARGET2_uptime_SS);
													fct_fprint_log_result = fct_fprint_log(log_text_size);
													if (fct_fprint_log_result != 0) {
														printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

														//	закрываем открытые handle списка процессов
														CloseHandle(process_tree_snapshot);
														CloseHandle(process_tree_snapshot2);
														return 5;
													}
												} else {
													printf("Process %s uptime: ", CONST_TARGET2_NAME);
													SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_TIMESTAMP);
													printf("%u:%02u:%02u\n\n", TARGET2_uptime_HH, TARGET2_uptime_MM, TARGET2_uptime_SS);
													SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_REGULAR);

													snprintf(log_text_size, sizeof(log_text_size), "Process %s uptime: %u:%02u:%02u\n", CONST_TARGET2_NAME, TARGET2_uptime_HH, TARGET2_uptime_MM, TARGET2_uptime_SS);
													fct_fprint_log_result = fct_fprint_log(log_text_size);
													if (fct_fprint_log_result != 0) {
														printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

														//	закрываем открытые handle списка процессов
														CloseHandle(process_tree_snapshot);
														CloseHandle(process_tree_snapshot2);
														return 5;
													}
												}

												TYPE_BYTE	path_AppData_local[MAX_PATH] = {0};
												TYPE_BYTE	path_exe[MAX_PATH] = {0};
												DWORD		EnvironmentVariable_len = GetEnvironmentVariableA("LOCALAPPDATA", path_AppData_local, sizeof(path_AppData_local));
												if (EnvironmentVariable_len == 0 || EnvironmentVariable_len >= sizeof(path_AppData_local)) {
													printf("ERROR: getting %%LOCALAPPDATA%% failed\n");

													snprintf(log_text_size, sizeof(log_text_size), "ERROR: getting %%LOCALAPPDATA%% failed\n");
													fct_fprint_log_result = fct_fprint_log(log_text_size);
													if (fct_fprint_log_result != 0) {
														printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

														//	закрываем открытые handle списка процессов
														CloseHandle(process_tree_snapshot);
														CloseHandle(process_tree_snapshot2);
														return 5;
													}

													//	закрываем открытые handle списка процессов
													CloseHandle(process_tree_snapshot);
													CloseHandle(process_tree_snapshot2);
													return 3;
												}

												snprintf(path_exe, sizeof(path_exe), "\"%s\\Microsoft\\prog.exe\"", path_AppData_local);

												STARTUPINFOA		sruct_STARTUPINFO = {0};
												PROCESS_INFORMATION	handle_PROC_INFO = {0};
												sruct_STARTUPINFO.cb = sizeof(sruct_STARTUPINFO);

												if (!CreateProcessA(NULL, path_exe, NULL, NULL, FALSE, 0, NULL, NULL, &sruct_STARTUPINFO, &handle_PROC_INFO)) {
													DWORD	error_CreateProcessA = GetLastError();
													wchar_t	error_CreateProcessA_message_w[256] = {0};

													DWORD	result_FormatMessageW = FormatMessageW(
														FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
														NULL,
														error_CreateProcessA,
														0,
														error_CreateProcessA_message_w,
														sizeof(error_CreateProcessA_message_w),
														NULL
													);

													if (result_FormatMessageW == 0) {
														printf("FormatMessageW failed: error code %lu\n", GetLastError());

														snprintf(log_text_size, sizeof(log_text_size), "FormatMessageW failed: error code %lu\n", GetLastError());
														fct_fprint_log_result = fct_fprint_log(log_text_size);
														if (fct_fprint_log_result != 0) {
															printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

															//	закрываем открытые handle
															CloseHandle(process_tree_snapshot);
															CloseHandle(process_tree_snapshot2);
															CloseHandle(handle_PROC_INFO.hProcess);
															CloseHandle(handle_PROC_INFO.hThread);
															return 5;
														}
													} else {
														TYPE_BYTE	CreateProcessA_message_866[256] = {0};
														TYPE_INT	result_WideCharToMultiByte = WideCharToMultiByte(
															866,
															0,
															error_CreateProcessA_message_w,
															-1,
															CreateProcessA_message_866,
															sizeof(CreateProcessA_message_866),
															NULL,
															NULL
														);

														if (result_WideCharToMultiByte == 0) {
															printf("WideCharToMultiByte failed: error code %lu\n", GetLastError());

															snprintf(log_text_size, sizeof(log_text_size), "WideCharToMultiByte failed: error code %lu\n", GetLastError());
															fct_fprint_log_result = fct_fprint_log(log_text_size);
															if (fct_fprint_log_result != 0) {
																printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

																//	закрываем открытые handle
																CloseHandle(process_tree_snapshot);
																CloseHandle(process_tree_snapshot2);
																CloseHandle(handle_PROC_INFO.hProcess);
																CloseHandle(handle_PROC_INFO.hThread);
																return 5;
															}
														} else {
															printf("CreateProcessA failed: %s", CreateProcessA_message_866);

															snprintf(log_text_size, sizeof(log_text_size), "CreateProcessA failed: %s", CreateProcessA_message_866);
															fct_fprint_log_result = fct_fprint_log(log_text_size);
															if (fct_fprint_log_result != 0) {
																printf("ERROR: logging failed [%u]\n", fct_fprint_log_result);

																//	закрываем открытые handle
																CloseHandle(process_tree_snapshot);
																CloseHandle(process_tree_snapshot2);
																CloseHandle(handle_PROC_INFO.hProcess);
																CloseHandle(handle_PROC_INFO.hThread);
																return 5;
															}
														}
													}

													//	закрываем открытые handle
													CloseHandle(process_tree_snapshot);
													CloseHandle(process_tree_snapshot2);
													CloseHandle(handle_PROC_INFO.hProcess);
													CloseHandle(handle_PROC_INFO.hThread);
													return 4;
												}

												CloseHandle(handle_PROC_INFO.hProcess);
												CloseHandle(handle_PROC_INFO.hThread);
												break;
											}
											//	закрываем открытый handle списка процессов
											CloseHandle(process_tree_snapshot2);
										}
										Sleep(sleep_value);
									}
								}
								//	завершаем перебор, т.к. нашли искомый PID
								break;
							}
						} while (Process32Next(process_tree_snapshot, &struct_process));	//	перебираем элементы списка процессов
					}
					//	закрываем открытый handle списка процессов
					CloseHandle(process_tree_snapshot);
				}
			}
			Sleep(sleep_value);
		}
	}
	return 0;
} else {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEF_COLOR_ERROR);
	printf("ERROR: cannot get Windows version\n");
	getchar();
	return 1;
}
}	// main(void)
