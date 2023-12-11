// Somewhat better with a touch of "worse"

#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <vector>
#include <unistd.h>
using namespace std;


namespace fs = std::filesystem;



vector<long> get_frequencies(const string path)
{
	long temp = 0;
	vector<long> frequencies;
	
	ifstream aval_freq_n(path.c_str());
	
	while(aval_freq_n >> temp)
		frequencies.push_back(temp);
	
	aval_freq_n.close();

	return frequencies;
}

bool do_the_shit(const string& cpu_type, int index)
{
	long freq = 0;
	auto aval_freq = get_frequencies("/sys/devices/system/cpu/cpu"+to_string(index)+"/cpufreq/scaling_available_frequencies");



	cout << cpu_type << endl;
	cout << "Available frequencies: ";
	for (auto& temp : aval_freq)
		cout << temp << "  ";
	cout << endl;



	bool flag;

	cout << "Enter frequency: ";
GO_BACK:
	cin >> freq;

	flag = true;
	for (auto& temp : aval_freq)
	{
		if (freq == temp)
		{
			flag = true;
			break;
		}
	}

	if (!flag)
	{
		cout << "\"" << freq << "\" is not one of the available freqency for this core type\n";
		cout << "Please enter one of the available freqency: ";
		goto GO_BACK;
	}




	// Set the CPU governer to userspace
	ofstream governor_n("/sys/devices/system/cpu/cpufreq/policy"+to_string(index)+"/scaling_governor");
	governor_n << "userspace";
	governor_n.close();



	ofstream scaling_setspeed_n("/sys/devices/system/cpu/cpu"+to_string(index)+"/cpufreq/scaling_setspeed");
	scaling_setspeed_n << freq;
	scaling_setspeed_n.close();

	return true;
}

int main(int argc, char *argv[])
{
	if(getuid() != 0)
	{
		cerr << "Please run this executable as root\n";
		return -1;
	}


	uint cpu_count = 0;
	fs::path cpu_param("/sys/devices/system/cpu");
	vector<long> aval_frequencies;
	
	// Determine the number of CPUs
	{
		regex pattern("^cpu[0-9]", regex_constants::grep);
		
		for (auto &ds : fs::directory_iterator(cpu_param))
		{
			if (regex_search(ds.path().filename().c_str() , pattern))
				cpu_count++;
		}
	}
	
	
	do_the_shit("Little Cores", 0);

	if (cpu_count > 4)
		do_the_shit("Big Cores", 4);
	
	return 0;
}
