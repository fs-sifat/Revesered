#include <iostream>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>

int fix_freq(const char* core_type, unsigned int aval_freq_count, unsigned int* aval_freq, const char* path)
{
	uint* ptr;
	uint temp;
	uint fix_freq = 0;
	printf("%s\n", core_type);

	if (aval_freq_count > 0)
	{
		ptr = aval_freq;
		temp = aval_freq_count;
		do {
			printf("%u\t", *ptr++);
			temp--;
		} while (temp != 0);
	}
	putchar('\n');

BACK_MF:

	printf("input fix freq: ");
	scanf("%d", &fix_freq);

	ptr = aval_freq;
	temp = aval_freq_count;
	bool flag = false;
	while(temp--)
	{
		if (*ptr++ == fix_freq)
		{
			flag = true;
			break;
		}
	}

	if (!flag)
	{
		printf("Enter available frequencies mf\n");
		goto BACK_MF;
		return -1;
	}

	FILE* setspeed_file = fopen(path, "w");
	if (setspeed_file == (FILE*)0x0)
	{
		fprintf(stderr, "fopen() | Failed to open file \"%s\"\n", path);
		return -1;
	}

	fprintf(setspeed_file, "%d", fix_freq);

	return 0;
}

int main()
{
	if (getuid() != 0)
	{
		fprintf(stderr, "Run this as root\n");
		return -1;
	}


	int temp;
	unsigned int cpu_count = 0;
	
	unsigned int aval_lit_freq_count = 0;
	unsigned int aval_lit_freq[40];

	unsigned int aval_big_freq_count = 0;
	unsigned int aval_big_freq[40];
	const char* cpu_param_path = "/sys/devices/system/cpu";



	DIR* cpu_param_dir = opendir(cpu_param_path);

	// Check if it exsts, if not complain and return
	if (cpu_param_dir == (DIR*)0x0)
	{
		fprintf(stderr, "opendir() | \"%s\" directory may not exists\n", cpu_param_path);
		return -1;
	}




	dirent* read_cpu_param = readdir(cpu_param_dir);


	// Check if it can be opened, if not complain and return
	if (read_cpu_param == (dirent*)0x0)
	{
		fprintf(stderr, "readdir() | Failed to read\"%s\"n", cpu_param_path);
		return -1;
	}


	// Determine the number of CPUs
	do
	{
		temp = fnmatch("cpu?", read_cpu_param->d_name, FNM_PATHNAME | FNM_PERIOD);
	
		if (temp == 0)
			cpu_count++;
		else if (temp != 1)
			fprintf(stderr, "fnmatch() | Error unknown file \"%s\"\n", read_cpu_param->d_name);
	
		read_cpu_param = readdir(cpu_param_dir);
	} while (read_cpu_param != (dirent*)0X0);

	closedir(cpu_param_dir);


	if (cpu_count < 0)
	{
		fprintf(stderr, "CPU count %u\n", cpu_count);
		return -1;
	}




	const char* lit_freq_path = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies";
	FILE* lit_freq_file = fopen(lit_freq_path, "r");

	// Check if it exists, if not complain and return

	if (lit_freq_file == (FILE*)0x0)
	{
		fprintf(stderr, "fopen() | Failed to open file %s\n", lit_freq_path);
		return -1;
	}


	unsigned int* ptr = aval_lit_freq;

	do
	{
		temp = fscanf(lit_freq_file, "%d", ptr++);
		
		if (temp != 1)
			break;

		aval_lit_freq_count++;
	} while (aval_lit_freq_count < 40);

	fclose(lit_freq_file);

	if (cpu_count > 4)
	{
		const char* big_freq_path = "/sys/devices/system/cpu/cpu4/cpufreq/scaling_available_frequencies";
		FILE* big_freq_file = fopen(big_freq_path, "r");

		// Check if it exists, if not complain and return

		if (big_freq_file == (FILE*)0x0)
		{
			fprintf(stderr, "fopen() | Failed to open file %s\n", big_freq_path);
			return -1;
		}


		unsigned int* ptr = aval_big_freq;

		do
		{
			temp = fscanf(big_freq_file, "%d", ptr++);

			if (temp != 1)
				break;

			aval_big_freq_count++;
		} while (aval_big_freq_count < 40);
		fclose(big_freq_file);
	}

	

	const char* lit_governer_path = "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor";
	FILE* lit_governer_file = fopen(lit_governer_path, "w");

	if (lit_governer_file == (FILE*)0x0)
	{
		fprintf(stderr, "fopen() | Unable to open file \"%s\"\n", lit_governer_path);
		return -1;
	}

	fwrite("userspace", 9, 1, lit_governer_file);

	fclose(lit_freq_file);

	int rv = fix_freq("Lit core", aval_lit_freq_count, aval_lit_freq, "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
	if (rv != 0)
		return -1;

	if (cpu_count > 4)
	{
		const char* big_governer_path = "/sys/devices/system/cpu/cpufreq/policy4/scaling_governor";
		FILE* big_governer_file = fopen(big_governer_path, "w");

		if (big_governer_file == (FILE*)0x0)
		{
			fprintf(stderr, "fopen() | Unable to open file \"%s\"\n", big_governer_path);
			return -1;
		}

		fwrite("userspace", 9, 1, big_governer_file);

		fclose(big_governer_file);

		int rv = fix_freq("Bigg core", aval_big_freq_count, aval_big_freq, "/sys/devices/system/cpu/cpu4/cpufreq/scaling_setspeed");
		if (rv != 0)
			return -1;
	}

	return 0;
}