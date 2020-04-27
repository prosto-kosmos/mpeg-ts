#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <iostream>
#include <vector>
#include <algorithm>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Specify parameter - file path!!!" << std::endl;
		std::cout << "\nPress any key to continue...";
		getch();
        return 0;
	}

	unsigned char byte(0);
	unsigned char left(0), right(0);
	short type(0);
	std::vector<short>pid;
	std::vector<FILE*>files;
	int period(188), num_file(0), flag_dop(0);

	std::vector<char>PackegeVideo;
	int SizePackege = 0;

	FILE *f, *v;
	f = fopen(argv[1], "rb");
	v = fopen("video.ips", "wb");
	fwrite("IP_STREAM", sizeof("IP_STREAM") - 1, 1, v);

	if (!f) {
		std::cout << "No find file: " << argv[1] << std::endl;
		return 0;
	}
	std::cout << "Open file: " << argv[1] << std::endl;

	fseek(f, 0, SEEK_END);

	int sz = ftell(f);
	std::cout << "Size file: " << sz << std::endl;
	std::cout << std::endl << "Please wait..." << std::endl;
	fseek(f, 0, SEEK_SET);

	for (int i = 0; i < sz; i++) {
		fread(&byte, sizeof(char), 1, f);
		if (byte == 0x47 && (i % period == 0)) {
			fread(&left, sizeof(char), 1, f);
			fread(&right, sizeof(char), 1, f);
			i += 2;
			type = ((left << 8) | right) & 0x1FFF; // выделяем PID - 13 byte

			// если в векторе типов пакета нет такого типа создаем новый файл
			if (std::find(pid.begin(), pid.end(), type) == pid.end()) {
				// std::cout << "Package types: " << type << std::endl;
				pid.push_back(type); // добавляем тип в вектор типов
				FILE *prog; // создаем новый файл
				char path[30];
				sprintf(path, "programm_%d.ips", num_file + 1);
				prog = fopen(path, "wb");
				fseek(prog, 0, SEEK_SET);
				fwrite("IP_STREAM", sizeof("IP_STREAM") - 1, 1, prog);
				files.push_back(prog); // добавляем файл в вектор файлов
				num_file += 1;
			}

			// ищем нужный файл
			for (int h = 0; h < pid.size(); h++) {
				if (pid[h] == type) { // h - индекс нужного файла
					FILE *prog = files[h]; // записываем пакет в нужный файл
					fwrite((char*)&period, 4, 1, prog);

					fwrite(&byte, sizeof(char), 1, prog);
					fwrite(&left, sizeof(char), 1, prog);
					fwrite(&right, sizeof(char), 1, prog);

					if (type == 0x100 && (left & 0x40)) {
						// сохраняем пакет видеоданных

						SizePackege = PackegeVideo.size();
						fwrite((char*)&SizePackege, 4, 1, v);
						for (int w = 0; w < SizePackege; w++) {
							fwrite((char*)&PackegeVideo[w], 1, 1, v);
						}
						PackegeVideo.clear();
					}

					for (int k = 3; k < period; k++) {
						i += 1;
						fread(&byte, sizeof(char), 1, f);
						fwrite(&byte, sizeof(char), 1, prog);
						if (type == 0x100 && k >= 3) {
							if (k == 3)
							{ // проверяем наличие дополнительного заголовка
								if (byte & 0x20) { // ищем его длину
									fread(&byte, sizeof(char), 1, f);
									fwrite(&byte, sizeof(char), 1, prog);
									k += 1;
									i += 1;
									flag_dop = (int)byte;
								}
								else {
									flag_dop = 0;
								}
								continue;
							}

							if (flag_dop == 0) {
								PackegeVideo.push_back(byte);
							}
							else {
								flag_dop -= 1;
							}

						}
					}

				}
			}
		}
	}
	int num = pid.size();
	std::cout << std::endl << "Number of programms: " << num << std::endl;

	// закрываем все файлы
	std::cout << "PID of programms: " << std::endl;
	for (int i = 0; i < files.size(); i++) {
		std::cout << " File \"programm_" << i +
			1 << ".ips\" PID: " << pid[i] << std::endl;
		fclose(files[i]);
	}
	std::cout << std::endl << "File \"video.ips\": Video file (PID=256)";
	fclose(f);
	fclose(v);
	std::cout << std::endl << "\nPress any key to continue...";
	getch();

	return 0;
}
