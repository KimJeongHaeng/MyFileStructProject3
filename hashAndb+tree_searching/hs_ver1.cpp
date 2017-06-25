#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <list>
#include <algorithm>
#include <vector>

using namespace std;
/* run this program using the console pauser or add your own getch, system("pause") or input loop */

//student's dataStruct
typedef struct InputData {
	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;	
}StudentData;

typedef struct InputProData {
	char name[20];
	unsigned int profID;
	int salary;
}ProfessorData;

class StuAndProFileStruct {
	private:
		int studentNum;
		fstream hashIO;
		fstream DBIO;
		vector <StudentData> tempStudentDatas;
		
		int professorNum;
		fstream hashIOPro;
		fstream DBIOPro;
		vector <ProfessorData> tempProfessorDatas;
		
	public:
		void fileHashAndDBOpen() {
			
			ofstream hashIO_temp;
			ofstream DBIO_temp;
			
			hashIO_temp.open("Students.hash", ios::out | ostream::binary);
			hashIO_temp.close();
				
			hashIO.open("Students.hash", ios::in | ios::out | ostream::binary);
			
			hashIO.seekp(0);
			int firstHashTablePrefix = 0;
			int hashPointBlockNum = 0;
			hashIO.write((char*)(&firstHashTablePrefix), sizeof(firstHashTablePrefix));
			hashIO.write((char*)(&hashPointBlockNum), sizeof(hashPointBlockNum));
			
			DBIO_temp.open("Student.DB", ios::out | ostream::binary);
			DBIO_temp.close();
			
			DBIO.open("Student.DB", ios::in | ios::out | ostream::binary);
			
		}
		
		void readStudTableAndUpdateFile() { //자료들을 한줄씩 읽음 
			ifstream readStudFile("sampleData.csv");
			//input student number by using getline
			string tmp_studentNum;
			getline(readStudFile, tmp_studentNum, '\n');
			studentNum = atoi(tmp_studentNum.c_str());
			
			StudentData currentStudData;
			int useHashingValue;
			int useHashingPrefix;
			int thisBlockNumber;
			//input all studentData by using getline
			
			string tmp_name, tmp_studentID, tmp_score, tmp_advisorID;
			while(getline(readStudFile, tmp_name,',')&&getline(readStudFile, tmp_studentID,',')&&getline(readStudFile, tmp_score,',')&&getline(readStudFile,tmp_advisorID,'\n')) {
				strcpy(currentStudData.name,tmp_name.c_str());
				if(tmp_name.length() >= 20) {
					string tmp_re_name = tmp_name.substr(0, 19);
					strcpy(currentStudData.name,tmp_re_name.c_str());
				}
				currentStudData.studentID = atoi(tmp_studentID.c_str());
				currentStudData.score = atof(tmp_score.c_str());
				currentStudData.advisorID = atoi(tmp_advisorID.c_str());
				
				hashIO.clear();
				hashIO.seekg(0);
				hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
				//useHashingPrefix = useHashingPrefix;
				
				useHashingValue = findHashValue(currentStudData.studentID, useHashingPrefix);
				thisBlockNumber = findBlockNumber(useHashingValue);
				
				inputDataInDB(currentStudData, thisBlockNumber);
				
				
			}
			hashTablePrint();
			
			readStudFile.close();
			hashIO.close();
			DBIO.close();
			
		}
		
		int findHashValue(unsigned int currentStudData_ID, int useHashingPrefix) { // 헤쉬테이블의 prefix이용 단, hashvalue가 "",0인경우는 같은 형식으로 취급 
			//hash파일과 DB파일은 계속 열려있어야함, hash파일과 DB파일이 update해주어야 함
			string hashingValue_string = hashingFunction(currentStudData_ID);
			//useHashingPrefix = 1;
			if(hashingValue_string.length() < useHashingPrefix) {
				while(hashingValue_string.length() != useHashingPrefix) {
					hashingValue_string = "0" + hashingValue_string;
				}
			}
			string useHashingValue_string;
			int useHashingValue_int;
			
			if(!hashingValue_string.compare("") == 0) {
				useHashingValue_string = hashingValue_string.substr(hashingValue_string.length() - useHashingPrefix, useHashingPrefix);
				useHashingValue_int = binaryToDecimal(useHashingValue_string);
			} else {
				useHashingValue_int = -1;
			}
			
			return useHashingValue_int;
		}
		
		string hashingFunction(unsigned int studentID) {
			
			if(studentID == 0) {
				return "0";
			} else if (studentID== 1) {
				return "1";
			} else if (studentID % 2 == 0) {
				return hashingFunction(studentID / 2) + "0";
			} else {
				return hashingFunction(studentID / 2) + "1";
			}
			
		}
		
		int binaryToDecimal(string useHashingValue_string) {
			int useHashingValue_int = 0, pow = 1;
			for(int i = useHashingValue_string.length() - 1 ;i >= 0 ; i--, pow <<= 1) {
				useHashingValue_int += (useHashingValue_string[i] - '0') * pow;
			}
			return useHashingValue_int;
		}
		
		int findBlockNumber(int useHashingValue) {
			int thisBlockNumber = 0;
			
			if(useHashingValue == -1) {
				useHashingValue == 0;
			}
			hashIO.clear();
			hashIO.seekg(4 + (4 * useHashingValue));
			hashIO.read((char*)(&thisBlockNumber), sizeof(thisBlockNumber));
			//cout << thisBlockNumber;
			return thisBlockNumber;
		}
		
		void inputDataInDB(StudentData currentStudData, int thisBlockNumber) {
			//cout << currentStudData.name << " ";
			//cout << sizeof(currentStudData) << " ";
			bool isOverflow = overflowCheck(thisBlockNumber, sizeof(currentStudData));
			
			if(!isOverflow) {
				inputData(currentStudData, thisBlockNumber);
				
			} else {
				solutionOverflow(currentStudData, thisBlockNumber);
			}
		}
		
		bool overflowCheck(int thisBlockNumber, int dataSize) {
			
			bool isOverflow = true;
			
			char isNull;
			DBIO.clear();
			DBIO.seekg((thisBlockNumber + 1) * 4096 - dataSize);
			
			DBIO.read((char*)(&isNull), sizeof(isNull));
			
			if(isNull == NULL || DBIO.tellg() == -1) {
				isOverflow = false;
			}
			
			return isOverflow;
		}
		
		void inputData(StudentData currentStudData, int thisBlockNumber) {			
			char isNull;
			int inputLocation = 0;
			
			for(int i = 0; i < 4096; i += sizeof(currentStudData)) {
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					inputLocation = i;
					
					break;
				}
			}
			
			DBIO.clear();
			DBIO.seekp(thisBlockNumber * 4096 + inputLocation);
			
			DBIO.write((char*)(&currentStudData.name), sizeof(currentStudData.name));
			DBIO.write((char*)(&currentStudData.studentID), sizeof(currentStudData.studentID));
			DBIO.write((char*)(&currentStudData.score), sizeof(currentStudData.score));
			DBIO.write((char*)(&currentStudData.advisorID), sizeof(currentStudData.advisorID));
			
		}
		
		void solutionOverflow(StudentData currentStudData, int thisBlockNumber) {
			//hashtable에서 현재 blocknumber와 같은것 찾음 - 함수만들기 
			int blockPointCount = findBlockPointCount(thisBlockNumber);
			//cout << blockPointCount << " ";
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			if(blockPointCount == 1) {
				reallocationOnePoint(currentStudData, thisBlockNumber); 
				//cout << "onepointover" <<endl; 
			} else {
				//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
				//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
				//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
				reallocationMultiPoint(currentStudData, thisBlockNumber);
				//cout << blockPointCount << "multipoint over" << endl;
			}
			
			
		}
		
		int findBlockPointCount(int thisBlockNumber) {
			int blockPointCount = 0;
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			//useHasingPrefix가 2의 지수승번으로  검사하면됨 
			int tempBlockNum;
			int howManyHashBlock = calculatePow(useHashingPrefix);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempBlockNum), sizeof(tempBlockNum));
				if(tempBlockNum == thisBlockNumber) {
					blockPointCount++;
				}
			}
			
			return blockPointCount;
		}
		
		int calculatePow(int twoIndex) {
			int tempTwoIndex = twoIndex;
			int powResult = 1;
			while(tempTwoIndex != 0) {
				powResult *= 2;
				tempTwoIndex --;
			}
			
			return powResult;
		}
		
		void reallocationOnePoint(StudentData currentStudData, int thisBlockNumber) {
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서(실재로 더 만들 필요는 없음 맨끝4k에 추가 관련 함수 필요) 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			int finalBlockNum;
			finalBlockNum = findFinalBlockNum();
			//cout << finalBlockNum;
			makeHashTableDouble();
			
			reallocateBlocksMulti(currentStudData, thisBlockNumber, finalBlockNum);
			reallocateHashTable(currentStudData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void makeHashTableDouble() {
			int beforeUseHashingPrefix;
			int afterUseHasingPrefix;
			vector <int> tempHashTable;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&beforeUseHashingPrefix), sizeof(beforeUseHashingPrefix));
			
			int howManyHashBlock = calculatePow(beforeUseHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				tempHashTable.push_back(tempHashTableNum);
			}
			
			hashIO.clear();
			hashIO.seekp(4 + howManyHashBlock * 4);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.write((char*)(&tempHashTable[i]), sizeof(tempHashTable[i]));
			}
			
			hashIO.clear();
			afterUseHasingPrefix = beforeUseHashingPrefix + 1;
			hashIO.seekp(0);
			hashIO.write((char*)(&afterUseHasingPrefix), sizeof(afterUseHasingPrefix));
			//testPrint();
			//cout << "endline \n";
			/*hashIO.clear();
			int temptest;
			hashIO.seekg(0);
			hashIO.read((char*)(&temptest), sizeof(temptest));
			cout << temptest << " ";
			cout << hashIO.tellg();*/
			
			
		}
		
		int findFinalBlockNum() {
			int finalBlockNum = 0;
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(finalBlockNum < tempHashTableNum) {
					finalBlockNum = tempHashTableNum;
				}
			}
			
			return finalBlockNum; 
		}
		
		void reallocateHashTable(StudentData currentStudData, int thisBlockNumber, int finalBlockNum) {
			int useHashingPrefix;
			int increaseFinalBlockNum = finalBlockNum + 1;
			vector <int> tempChangeHashTable;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			int changeHashPointNum = 0;
			int count = 0;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(thisBlockNumber == tempHashTableNum) {
					count++;
					if(changeHashPointNum == 0) {
						changeHashPointNum = 1;
					} else {
						tempChangeHashTable.push_back(i);
						changeHashPointNum = 0;
					}
					
				}
			}
			//cout << "현재 pointer갯수" << count << endl;
			for(int i = 0; i < tempChangeHashTable.size(); i++) {
				hashIO.clear();
				hashIO.seekp(4 + tempChangeHashTable[i] * 4);
				hashIO.write((char*)(&increaseFinalBlockNum), sizeof(increaseFinalBlockNum));
			}
			
			//testPrint();
			//cout << "test \n";
			
			
		}
		
		void reallocationMultiPoint(StudentData currentStudData, int thisBlockNumber) {
			//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
			//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입
			
			int finalBlockNum;
			finalBlockNum = findFinalBlockNum();
			//cout << finalBlockNum;
			
			
			reallocateBlocksMulti(currentStudData, thisBlockNumber, finalBlockNum);
			reallocateHashTable(currentStudData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void reallocateBlocksMulti(StudentData currentStudData, int thisBlockNumber, int finalBlockNum) {
			StudentData tempOneStudData;
			//StudentData emptyStudData;
			char isNull;
			char tempNullChar[20] = {NULL};
			unsigned int tempNullInt = NULL;
			float tempNullFloat = NULL;
			
			for(int i = 0; i < 4096; i += sizeof(currentStudData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				tempStudentDatas.push_back(tempOneStudData);
				
				DBIO.clear();
				DBIO.seekp(thisBlockNumber * 4096 + i);
				//cout << currentStudData.name << " ";
				DBIO.write((char*)(&tempNullChar), sizeof(tempNullChar));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				DBIO.write((char*)(&tempNullFloat), sizeof(tempNullFloat));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				
			}
			
			
			int useHashingPrefix;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int blockPointCount = findBlockPointCount(thisBlockNumber);
			
			int forRealusedPrefix = -1; //포인터의 개수가 1개면 안빼고 2개면 1빼고 4개면 2빼고 ... 
			while(blockPointCount != 1) {
				forRealusedPrefix ++;
				blockPointCount = blockPointCount/2;
				
			}
			
			int beforeHashValue = findHashValue(currentStudData.studentID, useHashingPrefix - 1 - forRealusedPrefix);
			
			for(int i = 0; i < tempStudentDatas.size(); i ++) {
				int newHashValue = findHashValue(tempStudentDatas[i].studentID, useHashingPrefix - forRealusedPrefix);
				if(newHashValue == beforeHashValue) {
					inputData(tempStudentDatas[i], thisBlockNumber);
				} else {
					inputData(tempStudentDatas[i], finalBlockNum + 1);
				}
			}
			
			tempStudentDatas.clear();
			
			int thisHashValue = findHashValue(currentStudData.studentID, useHashingPrefix - forRealusedPrefix);
			if(thisHashValue == beforeHashValue) {
				inputData(currentStudData, thisBlockNumber);
			} else {
				inputData(currentStudData, finalBlockNum + 1);
			}
				
		}
		
		
		
		void hashTablePrint() {
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			cout << "-----Student hashtable-----" << endl;
			cout << "hashTablePrefix :" << useHashingPrefix << endl;
			int tempHashTableNum;
		
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				cout << tempHashTableNum << endl;
			}
			
			cout << "-----hashtableEnd-----" << endl;
			
			/*int count = 0;
			for(int i = 0; i < 4096; i += 32) {
				char isNull;
				DBIO.clear();
				DBIO.seekg(i + 4096 * 31);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				StudentData tempOneStudData;
				cout << count << " ";
				count ++;
				
				DBIO.clear();
				DBIO.seekg(i);
				//DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				//DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				//DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				//DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				
				int useHashingPrefix;
				
				hashIO.clear();
				hashIO.seekg(0);
				hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
				
				//cout << findHashValue(tempOneStudData.studentID, useHashingPrefix) << " ";
				//cout << tempOneStudData.name << " ";
				//cout <<tempOneStudData.studentID << " ";
				//cout << tempOneStudData.score << " ";
				//cout << tempOneStudData.advisorID << "\n";
			}*/
		}
		
		//이위로는 학생 이 아래로는 교수 data부분-------------------------------------------------------------------------------------------------------- 
		
		void fileHashAndDBOpenPro() {
			
			ofstream hashIOPro_temp;
			ofstream DBIOPro_temp;
			
			hashIOPro_temp.open("Professors.hash", ios::out | ostream::binary);
			hashIOPro_temp.close();
				
			hashIOPro.open("Professors.hash", ios::in | ios::out | ostream::binary);
			
			hashIOPro.seekp(0);
			int firstHashTablePrefix = 0;
			int hashPointBlockNum = 0;
			hashIOPro.write((char*)(&firstHashTablePrefix), sizeof(firstHashTablePrefix));
			hashIOPro.write((char*)(&hashPointBlockNum), sizeof(hashPointBlockNum));
			
			DBIOPro_temp.open("Professors.DB", ios::out | ostream::binary);
			DBIOPro_temp.close();
			
			DBIOPro.open("Professors.DB", ios::in | ios::out | ostream::binary);
			
		}
		
		void readProTableAndUpdateFile() { //자료들을 한줄씩 읽음 
			ifstream readProFile("prof_data.csv");
			//input student number by using getline
			string tmp_professorNum;
			getline(readProFile, tmp_professorNum, '\n');
			professorNum = atoi(tmp_professorNum.c_str());
			
			ProfessorData currentProData;
			int useHashingValue;
			int useHashingPrefix;
			int thisBlockNumber;
			//input all studentData by using getline
			
			string tmp_name, tmp_profID, tmp_salary;
			while(getline(readProFile, tmp_name,',')&&getline(readProFile, tmp_profID,',')&&getline(readProFile,tmp_salary,'\n')) {
				strcpy(currentProData.name,tmp_name.c_str());
				if(tmp_name.length() >= 20) {
					string tmp_re_name = tmp_name.substr(0, 19);
					strcpy(currentProData.name,tmp_re_name.c_str());
				}
				currentProData.profID = atoi(tmp_profID.c_str());
				currentProData.salary = atoi(tmp_salary.c_str());
				
				hashIOPro.clear();
				hashIOPro.seekg(0);
				hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
				//useHashingPrefix = useHashingPrefix;
				
				useHashingValue = findHashValue(currentProData.profID, useHashingPrefix);
				thisBlockNumber = findBlockNumberPro(useHashingValue);
				
				inputDataInDBPro(currentProData, thisBlockNumber);
				
			}
			hashTablePrintPro();
			
			readProFile.close();
			hashIOPro.close();
			DBIOPro.close();
			
		}
		
		int findBlockNumberPro(int useHashingValue) {
			int thisBlockNumber = 0;
			
			if(useHashingValue == -1) {
				useHashingValue == 0;
			}
			hashIOPro.clear();
			hashIOPro.seekg(4 + (4 * useHashingValue));
			hashIOPro.read((char*)(&thisBlockNumber), sizeof(thisBlockNumber));
			//cout << thisBlockNumber;
			return thisBlockNumber;
		}
		
		void inputDataInDBPro(ProfessorData currentProData, int thisBlockNumber) {
			
			bool isOverflow = overflowCheckPro(thisBlockNumber, sizeof(currentProData));
			
			if(!isOverflow) {
				inputDataPro(currentProData, thisBlockNumber);
			
			} else {
				
				solutionOverflowPro(currentProData, thisBlockNumber);
			}
		}
		
		bool overflowCheckPro(int thisBlockNumber, int dataSize) {
			
			bool isOverflow = true;
			
			char isNull;
			DBIOPro.clear();
			DBIOPro.seekg((thisBlockNumber + 1) * 4096 - dataSize - 8);
			
			DBIOPro.read((char*)(&isNull), sizeof(isNull));
			
			if(isNull == NULL || DBIOPro.tellg() == -1) {
				isOverflow = false;
			}
			
			return isOverflow;
		}
		
		void inputDataPro(ProfessorData currentProData, int thisBlockNumber) {			
			char isNull;
			int inputLocation;
			
			for(int i = 0; i < 4096; i += sizeof(currentProData)) {
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					inputLocation = i;
					//cout << i << " ";
					break;
				}
			}
			
			DBIOPro.clear();
			DBIOPro.seekp(thisBlockNumber * 4096 + inputLocation);
			//cout << currentStudData.name << " ";
			DBIOPro.write((char*)(&currentProData.name), sizeof(currentProData.name));
			DBIOPro.write((char*)(&currentProData.profID), sizeof(currentProData.profID));
			DBIOPro.write((char*)(&currentProData.salary), sizeof(currentProData.salary));
			
		}
		
		void solutionOverflowPro(ProfessorData currentProData, int thisBlockNumber) {
			//hashtable에서 현재 blocknumber와 같은것 찾음 - 함수만들기 
			int blockPointCount = findBlockPointCountPro(thisBlockNumber);
			//cout << blockPointCount << " ";
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			if(blockPointCount == 1) {
				reallocationOnePointPro(currentProData, thisBlockNumber); 
				//cout << "onepointover" <<endl; 
			} else {
				//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
				//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
				//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
				reallocationMultiPointPro(currentProData, thisBlockNumber);
				//cout << blockPointCount << "multipoint over" << endl;
			}
			
			
		}
		
		int findBlockPointCountPro(int thisBlockNumber) {
			int blockPointCount = 0;
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			//useHasingPrefix가 2의 지수승번으로  검사하면됨 
			int tempBlockNum;
			int howManyHashBlock = calculatePow(useHashingPrefix);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempBlockNum), sizeof(tempBlockNum));
				if(tempBlockNum == thisBlockNumber) {
					blockPointCount++;
				}
			}
			
			return blockPointCount;
		}
		
		void reallocationOnePointPro(ProfessorData currentProData, int thisBlockNumber) {
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서(실재로 더 만들 필요는 없음 맨끝4k에 추가 관련 함수 필요) 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			int finalBlockNum;
			finalBlockNum = findFinalBlockNumPro();
			//cout << finalBlockNum;
			makeHashTableDoublePro();
			
			reallocateBlocksMultiPro(currentProData, thisBlockNumber, finalBlockNum);
			reallocateHashTablePro(currentProData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void makeHashTableDoublePro() {
			int beforeUseHashingPrefix;
			int afterUseHasingPrefix;
			vector <int> tempHashTable;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&beforeUseHashingPrefix), sizeof(beforeUseHashingPrefix));
			
			int howManyHashBlock = calculatePow(beforeUseHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				tempHashTable.push_back(tempHashTableNum);
			}
			
			hashIOPro.clear();
			hashIOPro.seekp(4 + howManyHashBlock * 4);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.write((char*)(&tempHashTable[i]), sizeof(tempHashTable[i]));
			}
			
			hashIOPro.clear();
			afterUseHasingPrefix = beforeUseHashingPrefix + 1;
			hashIOPro.seekp(0);
			hashIOPro.write((char*)(&afterUseHasingPrefix), sizeof(afterUseHasingPrefix));
			
			
		}
		
		int findFinalBlockNumPro() {
			int finalBlockNum = 0;
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(finalBlockNum < tempHashTableNum) {
					finalBlockNum = tempHashTableNum;
				}
			}
			//cout << "마지막 block : " << finalBlockNum << endl;
			return finalBlockNum; 
		}
		
		
		void reallocateHashTablePro(ProfessorData currentProData, int thisBlockNumber, int finalBlockNum) {
			int useHashingPrefix;
			int increaseFinalBlockNum = finalBlockNum + 1;
			vector <int> tempChangeHashTable;
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			int changeHashPointNum = 0;
			int count = 0;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(thisBlockNumber == tempHashTableNum) {
					count++;
					if(changeHashPointNum == 0) {
						changeHashPointNum = 1;
					} else {
						tempChangeHashTable.push_back(i);
						changeHashPointNum = 0;
					}
					
				}
			}
			//cout << "현재 pointer갯수" << count << endl;
			for(int i = 0; i < tempChangeHashTable.size(); i++) {
				hashIOPro.clear();
				hashIOPro.seekp(4 + tempChangeHashTable[i] * 4);
				hashIOPro.write((char*)(&increaseFinalBlockNum), sizeof(increaseFinalBlockNum));
			}
			
			
			
		}
		
		void reallocationMultiPointPro(ProfessorData currentProData, int thisBlockNumber) {
			//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
			//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입
			
			int finalBlockNum;
			finalBlockNum = findFinalBlockNumPro();
			//cout << finalBlockNum;
			
			reallocateBlocksMultiPro(currentProData, thisBlockNumber, finalBlockNum);
			reallocateHashTablePro(currentProData, thisBlockNumber, finalBlockNum);
			
		}
		
		void reallocateBlocksMultiPro(ProfessorData currentProData, int thisBlockNumber, int finalBlockNum) {
			ProfessorData tempOneProData;
			//StudentData emptyStudData;
			char isNull;
			char tempNullChar[20] = {NULL};
			unsigned int tempNullInt = NULL;
			int tempNullInt_sign = NULL;
			
			
			for(int i = 0; i < 4096; i += sizeof(currentProData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					break;
				}
				
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&tempOneProData.name), sizeof(tempOneProData.name));
				DBIOPro.read((char*)(&tempOneProData.profID), sizeof(tempOneProData.profID));
				DBIOPro.read((char*)(&tempOneProData.salary), sizeof(tempOneProData.salary));
				tempProfessorDatas.push_back(tempOneProData);
				
				DBIOPro.clear();
				DBIOPro.seekp(thisBlockNumber * 4096 + i);
				//cout << currentStudData.name << " ";
				DBIOPro.write((char*)(&tempNullChar), sizeof(tempNullChar));
				DBIOPro.write((char*)(&tempNullInt), sizeof(tempNullInt));
				DBIOPro.write((char*)(&tempNullInt_sign), sizeof(tempNullInt_sign));
				
			}
			
			int useHashingPrefix;
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int blockPointCount = findBlockPointCountPro(thisBlockNumber);
		
			int forRealusedPrefix = -1; //포인터의 개수가 1개면 안빼고 2개면 1빼고 4개면 2빼고 ... 
			while(blockPointCount != 1) {
				forRealusedPrefix ++;
				blockPointCount = blockPointCount/2;
				
			}
			
			int beforeHashValue = findHashValue(currentProData.profID, useHashingPrefix - 1 - forRealusedPrefix);
			
			for(int i = 0; i < tempProfessorDatas.size(); i ++) {
				int newHashValue = findHashValue(tempProfessorDatas[i].profID, useHashingPrefix - forRealusedPrefix);
				if(newHashValue == beforeHashValue) {
					inputDataPro(tempProfessorDatas[i], thisBlockNumber);
				} else {
					inputDataPro(tempProfessorDatas[i], finalBlockNum + 1);
				}
			}
			
			tempProfessorDatas.clear();
			
			int thisHashValue = findHashValue(currentProData.profID, useHashingPrefix - forRealusedPrefix);
			if(thisHashValue == beforeHashValue) {
				inputDataPro(currentProData, thisBlockNumber);
			} else {
				inputDataPro(currentProData, finalBlockNum + 1);
			}
				
		}
		
		
		
		void hashTablePrintPro() {
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			cout << "-----Professor hashtable-----" << endl;
			cout << "hashTablePrefix :" << useHashingPrefix << endl;
			int tempHashTableNum;
		
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				cout << tempHashTableNum << endl;
			}
			
			cout << "-----hashtableEnd-----" << endl;
			
			/*int count = 0;
			for(int i = 0; i < 4096; i += 28) {
				char isNull;
				DBIOPro.clear();
				DBIOPro.seekg(i + 4096 * 1);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					break;
				}
				ProfessorData tempOneProData;
				cout << count << " ";
				count ++;
				
				DBIOPro.clear();
				DBIOPro.seekg(i + 4096 * 1);
				DBIOPro.read((char*)(&tempOneProData.name), sizeof(tempOneProData.name));
				DBIOPro.read((char*)(&tempOneProData.profID), sizeof(tempOneProData.profID));
				DBIOPro.read((char*)(&tempOneProData.salary), sizeof(tempOneProData.salary));
				
				int useHashingPrefix;
				
				hashIOPro.clear();
				hashIOPro.seekg(0);
				hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
				
				cout << findHashValue(tempOneProData.profID, useHashingPrefix) << " ";
				cout << tempOneProData.name << " ";
				cout <<tempOneProData.profID << " ";
				cout << tempOneProData.salary << "\n";
			}*/
		}
		
		/*void readQueryFile() {
			
		}*/
		
};

int main(int argc, char** argv) {
	StuAndProFileStruct stuAndProFS;
	stuAndProFS.fileHashAndDBOpen();
	stuAndProFS.readStudTableAndUpdateFile();
	stuAndProFS.fileHashAndDBOpenPro();
	stuAndProFS.readProTableAndUpdateFile();
	//stuAndProFS.readQueryFile();
	
	//일단 query파일을 불러와서 조건에 따라 함수수행 .. match / range/ join
	 
	return 0;
}
