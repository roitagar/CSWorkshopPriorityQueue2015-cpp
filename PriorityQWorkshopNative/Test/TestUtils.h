#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

string timeStamp;

void saveResult(string* result, int count){
	int i;

	if(timeStamp.empty()){
		std::time_t rawtime;
		std::tm* timeinfo;
		std::time(&rawtime);
		timeinfo = std::localtime(&rawtime);

		ostringstream stringStream;
		stringStream 					<< 1900 +	timeinfo->tm_year;
		stringStream << setfill('0')<<setw(2)<< 1 +	timeinfo->tm_mon;
		stringStream << setfill('0')<<setw(2)<< 	timeinfo->tm_mday;
		stringStream << "_";
		stringStream << setfill('0')<<setw(2)<< 	timeinfo->tm_hour;
		stringStream << setfill('0')<<setw(2)<< 	timeinfo->tm_min;
		stringStream << setfill('0')<<setw(2)<< 	timeinfo->tm_sec;
		timeStamp = stringStream.str();

		//			timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(Calendar.getInstance().getTime());
	}

	std::stringstream buff;
	//		StringBuffer buff = new StringBuffer();
	for(i=0;i<count;i++)
		//		for(String s:result)
	{
		buff<<result[i]<<'\t';
		//			buff.append(s);
		//			buff.append("\t");
	}

	string fileName = "results_"+ timeStamp +".txt";
	ofstream output;
	output.open(fileName, fstream::app);
	//		output = new BufferedWriter(new FileWriter(fileName, true));
	//		output.append(buff.toString() + "\n");
	output << buff.str()<<endl;
	cout << buff.str()<<endl;
	//		System.out.println(buff.toString());
	//		output.flush();
	//		output.close();

	//		System.out.println("Error in print result to file");

}
