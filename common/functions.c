// === include ===
#include <stdio.h>

#include <windows.h>
#include <time.h>

#include "functions.h"
#include "types.h"

// === defines ===
#define	log_file_name	"log"
#define	log_file_name_old	"log_old"

// === code ===
void	fct_throbber(void) {
	// === declaration ===
	static TYPE_USINT	throbber_frame = 0;
	const TYPE_BYTE		throbber_text[] = {0xb3, 0x2f, 0xc4, 0x5c};

	// === code ===
	printf("\rWaiting... %c", throbber_text[throbber_frame & 0x3]);
	fflush(stdout);
	throbber_frame = throbber_frame + 1;
}


void	fct_print_local_time(void) {
	// === declaration ===
	time_t local_time;
	struct tm *struct_local_time;

	// === code ===
	time(&local_time);
	struct_local_time = localtime(&local_time);

	printf("[%04d.%02d.%02d %02d:%02d:%02d]\n",
				struct_local_time->tm_year + 1900,
				struct_local_time->tm_mon + 1,
				struct_local_time->tm_mday,
				struct_local_time->tm_hour,
				struct_local_time->tm_min,
				struct_local_time->tm_sec);
}


void	fct_parse_numeric_argument(const TYPE_BYTE *pointer_BYTE, const TYPE_UINT value_MIN, const TYPE_UINT value_MAX, TYPE_UINT *OUT_value) {
	TYPE_UINT	tmp_value = 0;
	TYPE_USINT	tmp_digit = 0;

	//	пока значение по адресу указателя ненулевое (не конец строки)
	while (*pointer_BYTE) {
		//	если значение байта соответствует ASCII-коду цифры
		if (*pointer_BYTE >= 0x30 && *pointer_BYTE <= 0x39) {
			//	превращаем ASCII-код в цифру
			tmp_digit = *pointer_BYTE & 0x0F;
			//	проверка на переполнение переменной при последующем умножении
			if (tmp_value > (0xffffU - tmp_digit) / 10) {
				printf("ERROR: numeric overflow\n");
				return;
			}

			//	прибавляем к временному значению новую цифру со смещением предыдущего результата на порядок
			tmp_value = tmp_value * 10 + tmp_digit;
			if (tmp_value > value_MAX) {
				printf("ERROR: numeric argument out of range %d..%d\n", value_MIN, value_MAX);
				return;
			}
		} else {
			printf("ERROR: non-numeric argument\n");
			return;
		}
		//	двигаем указатель по строке далее
		pointer_BYTE = pointer_BYTE + 1;
	}

	if (tmp_value < value_MIN) {
		printf("ERROR: numeric argument out of range %d..%d\n", value_MIN, value_MAX);
		return;
	} else {
		*OUT_value = tmp_value;
		return;
	}
}


TYPE_BYTE	fct_fprint_log(const TYPE_BYTE *string_to_log) {
	// === declaration ===
	FILE *handle_log_file;
	ULARGE_INTEGER	struct_bytes_ttl, struct_bytes_ttl_free, struct_bytes_avail_free;

	// === code ===
	//	запрашиваем у ОС свободное место на диске
	if (GetDiskFreeSpaceExA(".", &struct_bytes_avail_free, &struct_bytes_ttl, &struct_bytes_ttl_free)) {
		//	если свободного места меньше - отключаемся
		if (struct_bytes_avail_free.QuadPart < 100ULL * 1024 * 1024) {
			printf("WARNING: not enought free space for log file\n");
			printf("Disk space total: %llu MB\n", struct_bytes_ttl.QuadPart / (1024 * 1024));
			printf("Disk space free available: %llu MB\n", struct_bytes_avail_free.QuadPart / (1024 * 1024));
			printf("Disk space free total: %llu MB\n", struct_bytes_ttl_free.QuadPart / (1024 * 1024));
			return 12;
		} else {
			//	открываем файл лога на чтение
			handle_log_file = fopen(log_file_name, "r");
			if (handle_log_file == NULL) {
				perror("ERROR: log file opening failed");
				return 13;
			} else {
				//	получаем размер файла
				size_t	log_file_size = 0;
				fseek(handle_log_file, 0, SEEK_END);
				log_file_size = ftell(handle_log_file);
				fclose(handle_log_file);

				//	если размер файла превысил заданный
				if (log_file_size > 100ULL * 1024) {
					printf("WARNING: file size too big - %u bytes\ntrying to create new one...\n", log_file_size);
					//	удаляем старый файл лога
					if (remove(log_file_name_old) == -1) {
						perror("ERROR: old log file removing failed");
					}

					//	и переименовываем актуальный в "старый"
					if (rename(log_file_name, log_file_name_old) == 0) {
						printf("Rename %s to %s done\n", log_file_name, log_file_name_old);
					} else {
						perror("ERROR: rename failed");
						return 14;
					}
				}
			}
		}
	} else {
		printf("ERROR: check free space failed: %lu\n", GetLastError());
		return 11;
	}

	//	открываем файл лога на добавление
	handle_log_file = fopen(log_file_name, "a");
	if (handle_log_file == NULL) {
		perror("ERROR: log file opening failed");
		return 15;
	} else {
		//	получаем системное время
		time_t local_time;
		struct tm *struct_local_time;

		time(&local_time);
		struct_local_time = localtime(&local_time);

		//	добавляем в файл метку времени
		fprintf(handle_log_file, "[%04d.%02d.%02d %02d:%02d:%02d] ",
				struct_local_time->tm_year + 1900,
				struct_local_time->tm_mon + 1,
				struct_local_time->tm_mday,
				struct_local_time->tm_hour,
				struct_local_time->tm_min,
				struct_local_time->tm_sec);

		//	и сообщение
		fprintf(handle_log_file, "%s", string_to_log);
		fclose(handle_log_file);
	}
	return 0;
}
